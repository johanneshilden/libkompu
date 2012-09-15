#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include "lcalc.h"

static void
dump_term_tree(struct lambda_term *term)
{
    switch (term->type)
    {
    case LAMBDA_TERM_APPLICATION:
        printf("(");
        dump_term_tree(term->app.expr1);
        dump_term_tree(term->app.expr2);
        printf(")");
        break;
    case LAMBDA_TERM_ABSTRACTION:
        printf("\\");
        printf("%i.", term->abstr.var);
        printf("(");
        dump_term_tree(term->abstr.expr);
        printf(")");
        break;
    case LAMBDA_TERM_VARIABLE:
        printf("%i ", term->var);
        break;
    } /* end switch */
}

static void
lambda_term_traverse(struct lambda_term *term, struct buf *b, struct id_pair **list, int n)
{
    static char tmpstr[20];
    struct id_pair *pair, *next;
    lambda_id id;

    switch (term->type)
    {
    case LAMBDA_TERM_APPLICATION:
        buf_append_chars(b, "[");
        lambda_term_traverse(term->app.expr1, b, list, n);
        buf_append_chars(b, ",");
        lambda_term_traverse(term->app.expr2, b, list, n);
        buf_append_chars(b, "]");
        break;
    case LAMBDA_TERM_ABSTRACTION:
        buf_append_chars(b, "L");
        snprintf(tmpstr, 20, "%u", n);
        pair = malloc(sizeof(struct id_pair));
        pair->next = *list;
        pair->key  = term->abstr.var;
        pair->val  = n++;
        *list = pair;
        buf_append_chars(b, tmpstr);
        buf_append_chars(b, ".");
        lambda_term_traverse(term->abstr.expr, b, list, n);
        break;
    case LAMBDA_TERM_VARIABLE:
        buf_append_chars(b, "V");
        id = term->abstr.var;
        next = *list;
        while (next) {
            if (term->abstr.var == next->key) {
                id = next->val;
                break;
            }
            next = next->next;
        }
        snprintf(tmpstr, 20, "%u", id);
        buf_append_chars(b, tmpstr);
        break;
    } /* end switch */
}

static void
lambda_term_alpha_hash(struct lambda_term *term, struct buf *b)
{
    struct id_pair *list, *next;
    list = NULL;
    lambda_term_traverse(term, b, &list, 0);

    while (list) {
        next = list->next;
        free(list);
        list = next;
    }
}

struct
lambda_term *lambda_application_new(struct lambda_term *e1, struct lambda_term *e2)
{
    struct lambda_term *term;
    term = malloc(sizeof(struct lambda_term));
    term->type = LAMBDA_TERM_APPLICATION;
    term->app.expr1 = e1;
    term->app.expr2 = e2;
    return term;
}

struct
lambda_term *lambda_abstraction_new(lambda_id v, struct lambda_term *e)
{
    struct lambda_term *term;
    term = malloc(sizeof(struct lambda_term));
    term->type = LAMBDA_TERM_ABSTRACTION;
    term->abstr.var = v;
    term->abstr.expr = e;
    return term;
}

struct
lambda_term *lambda_variable_new(lambda_id v)
{
    struct lambda_term *term;
    term = malloc(sizeof(struct lambda_term));
    term->type = LAMBDA_TERM_VARIABLE;
    term->var = v;
    return term;
}

void
lambda_term_destroy(struct lambda_term *term)
{
    if (!term)
        return;

    switch (term->type)
    {
    case LAMBDA_TERM_APPLICATION:
        lambda_term_destroy(term->app.expr1);
        lambda_term_destroy(term->app.expr2);
        break;
    case LAMBDA_TERM_ABSTRACTION:
        lambda_term_destroy(term->abstr.expr);
        break;
    case LAMBDA_TERM_VARIABLE:
        break;
    } /* end switch */

    free(term);
}

struct lambda_term *
lambda_term_clone(struct lambda_term *term)
{
    assert(term);
    switch (term->type)
    {
    case LAMBDA_TERM_APPLICATION:
        return lambda_application_new(lambda_term_clone(term->app.expr1),
                                      lambda_term_clone(term->app.expr2));
    case LAMBDA_TERM_ABSTRACTION:
        return lambda_abstraction_new(term->abstr.var, lambda_term_clone(term->abstr.expr));
    case LAMBDA_TERM_VARIABLE:
        return lambda_variable_new(term->var);
    } /* end switch */

    /*
     *  If we reach here we're in trouble.
     */
    return NULL;
}

void
lambda_term_substitute(struct lambda_term **term, lambda_id x, struct lambda_term *n)
{
    switch ((*term)->type)
    {
    case LAMBDA_TERM_APPLICATION:
        lambda_term_substitute(&(*term)->app.expr1, x, n);
        lambda_term_substitute(&(*term)->app.expr2, x, n);
        break;
    case LAMBDA_TERM_ABSTRACTION:
        if (x != (*term)->abstr.var)
            lambda_term_substitute(&(*term)->abstr.expr, x, n);
        break;
    case LAMBDA_TERM_VARIABLE:
        if (x == (*term)->var) {
            lambda_term_destroy(*term);
            *term = lambda_term_clone(n);
        }
        break;
    } /* end switch */
}

/*!
 *  Performs a beta reduction step on \a redex, if the term is a valid
 *  redex. Otherwise does nothing.
 */
void
lambda_redex_beta_reduce(struct lambda_term **redex)
{
    struct lambda_term *e1, *e2, *t;
    lambda_id v;

    if ((*redex)->type != LAMBDA_TERM_APPLICATION
     || (*redex)->app.expr1->type != LAMBDA_TERM_ABSTRACTION) {
        /*
         *  Expression is not a redex.
         */
        return;
    }

    e1 = (*redex)->app.expr1;
    e2 = lambda_term_clone((*redex)->app.expr2);
    v  = e1->abstr.var;
    t  = lambda_term_clone(e1->abstr.expr);

    lambda_term_destroy(*redex);
    *redex = t;
    lambda_term_substitute(redex, v, e2);
    lambda_term_destroy(e2);
}

/*!
 *  The leftmost, outermost redex is always reduced first. That is, whenever
 *  possible the arguments are substituted into the body of an abstraction
 *  before the arguments are reduced.
 */
void
lambda_term_normal_order_reduce_step(struct lambda_term **term)
{
    switch ((*term)->type)
    {
    case LAMBDA_TERM_APPLICATION:
        lambda_redex_beta_reduce(term);
        if (LAMBDA_TERM_APPLICATION == (*term)->type) {
            lambda_term_normal_order_reduce_step(&(*term)->app.expr1);
            lambda_term_normal_order_reduce_step(&(*term)->app.expr2);
        }
        break;
    case LAMBDA_TERM_ABSTRACTION:
        lambda_term_normal_order_reduce_step(&(*term)->abstr.expr);
    case LAMBDA_TERM_VARIABLE:
        break;
    } /* end switch */
}

/*!
 *  As normal order, but no reductions are performed inside abstractions.
 *  For example λx.(λx.x)x is in normal form according to this strategy,
 *  although it contains the redex (λx.x)x.
 */
void
lambda_term_call_by_name_reduce_step(struct lambda_term **term)
{
    switch ((*term)->type)
    {
    case LAMBDA_TERM_APPLICATION:
        /*
         *  An application (e1 e2) is reduced by first reducing e1 to e1'.
         */
        lambda_term_call_by_name_reduce_step(&(*term)->app.expr1);

        if (LAMBDA_TERM_ABSTRACTION == (*term)->app.expr1->type) {
            /*
             *  If e1' is an abstraction (λx.e), then (e1' e2) is a redex,
             *  which can be reduced by performing substitution e[x := N].
             */

            lambda_redex_beta_reduce(term);
        } else {
            /*
             *  If e1' is not an abstraction, e2 is reduced to e2'.
             */

            lambda_term_call_by_name_reduce_step(&(*term)->app.expr2);
        }
        break;
    case LAMBDA_TERM_ABSTRACTION:
        /*
         *  An abstraction is in whnf and reduces to itself.
         */
    case LAMBDA_TERM_VARIABLE:
        /*
         *  A variable is in whnf and reduces to itself.
         */
        break;
    } /* end switch */
}

/*!
 *  Tests two terms for alpha-convertibility. Terms that differ only by
 *  alpha-conversion (renaming of bound variables) are called α-equivalent.
 *
 *  Returns 1 if the two terms are α-equivalent, otherwise 0.
 */
uint8_t
lambda_term_alpha_compare(struct lambda_term *t1, struct lambda_term *t2)
{
    struct buf *b1, *b2;
    uint8_t r;

    b1 = buf_new(64);
    b2 = buf_new(64);
    lambda_term_alpha_hash(t1, b1);
    lambda_term_alpha_hash(t2, b2);

    printf("#1:%s\n", b1->data);
    printf("#2:%s\n", b2->data);

    r = buf_compare(b1, b2);
    buf_destroy(b1);
    buf_destroy(b2);
    return r;
}

void
lambda_term_dump(struct lambda_term *term)
{
    assert(term);
    dump_term_tree(term);
    printf("\n");
}
