#ifndef LCALC_H
#define LCALC_H

#include <inttypes.h>

#define lambda_id uint16_t

typedef enum {
    LAMBDA_TERM_VARIABLE = 0,
    LAMBDA_TERM_APPLICATION,
    LAMBDA_TERM_ABSTRACTION
} lambda_term_type_t;

struct lambda_application
{
    struct lambda_term *expr1;
    struct lambda_term *expr2;
};

struct lambda_abstraction
{
    lambda_id var;
    struct lambda_term *expr;
};

struct lambda_term
{
    lambda_term_type_t type;
    union {
        struct lambda_application app;
        struct lambda_abstraction abstr;
        lambda_id var;
    };
};

struct lambda_term *lambda_application_new(struct lambda_term *e1, struct lambda_term *e2);
struct lambda_term *lambda_abstraction_new(lambda_id v, struct lambda_term *e);
struct lambda_term *lambda_variable_new(lambda_id v);

void lambda_term_destroy(struct lambda_term *term);
struct lambda_term *lambda_term_clone(struct lambda_term *term);

void lambda_term_substitute(struct lambda_term **term, lambda_id x, struct lambda_term *n);
void lambda_redex_beta_reduce(struct lambda_term **redex);

void lambda_term_normal_order_reduce_step(struct lambda_term **term);
void lambda_term_call_by_name_reduce_step(struct lambda_term **term);

void lambda_term_alpha_hash(struct lambda_term *term);

void lambda_term_dump(struct lambda_term *term);

#endif /* LCALC_H */
