#include "comp.h"

#include <string.h>
#include <malloc.h>
#include <assert.h>

/*
 * A partial function is general recursive if it can be built up from the
 * initial zero, successor, and projection functions, by use of composition,
 * primitive recursion, and search (i.e., the μ-operator).
 *
 * The class of general recursive partial functions on N is (as Turing proved)
 * exactly the same as the class of Turing computable partial functions.
 */

/*!
    \struct node

    \brief The base node struct, holding a pointer to node specific data.
 */

/*!
    \struct node_projection

    \brief A leaf projection node.
 */

/*!
    \struct node_composition

    \brief Node representing an application of composition.
 */

/*!
    \struct node_recursion

    \brief Node representing an application of primitive recursion.
 */

/*!
    \struct node_search

    \brief Search operator node.
 */

/*!
    Creates a new projection function node.
 */
struct node *
projection_node_new(int place)
{
    struct node *n;
    struct node_projection *proj;
    n = malloc(sizeof(struct node));
    proj = malloc(sizeof(struct node_projection));
    n->type = NODE_PROJECTION;
    proj->place = place;
    n->data = proj;
    return n;
}

/*!
    Creates a new zero function node.
 */
struct node *
zero_node_new()
{
    struct node *n;
    n = malloc(sizeof(struct node));
    n->data = NULL;
    n->type = NODE_ZERO;
    return n;
}

/*!
    Creates a new successor function node.
 */
struct node *
successor_node_new()
{
    struct node *n;
    n = malloc(sizeof(struct node));
    n->data = NULL;
    n->type = NODE_SUCCESSOR;
    return n;
}

/*!
    Creates a new composition node.

    Explicit transformation permits scrambling variables, repeating variables,
    omitting variables, and substituting constants.
 */
struct node *
composition_node_new(struct node *f, struct node **g)
{
    struct node *n, *d;
    struct node_composition *comp;
    n = malloc(sizeof(struct node));
    comp = malloc(sizeof(struct node_composition));
    n->type = NODE_COMPOSITION;
    comp->f = f;
    comp->g = g;
    n->data = comp;
    while (g && (d = *g))
        ++g;
    comp->places = (g - comp->g);
    return n;
}

/*!
    Creates a new recursion node.
 */
struct node *
recursion_node_new(struct node *f, struct node *g)
{
    struct node *n;
    struct node_recursion *rec;
    n = malloc(sizeof(struct node));
    rec = malloc(sizeof(struct node_recursion));
    n->type = NODE_RECURSION;
    rec->f = f;
    rec->g = g;
    n->data = rec;
    return n;
}

/*!
    Creates a new search node (the μ-operator).
 */
struct node *
search_node_new(struct node *p)
{
    struct node *n;
    struct node_search *search;
    n = malloc(sizeof(struct node));
    search = malloc(sizeof(struct node_search));
    n->type = NODE_SEARCH;
    search->p = p;
    n->data = search;
    return n;
}

/*!
    Destroys the provided node and releases associated memory.
 */
void
node_destroy(struct node *n)
{
    assert(n);

    switch (n->type)
    {
    case NODE_COMPOSITION:
    {
        struct node **curr;
        struct node_composition *comp;
        int i;
        comp = (struct node_composition *) n->data;
        curr = comp->g;
        i = comp->places;
        while (i--) {
            node_destroy(*curr);
            ++curr;
        }
        free(comp->g);
        node_destroy(comp->f);
        break;
    }
    case NODE_RECURSION:
    {
        struct node_recursion *rec;
        rec = (struct node_recursion *) n->data;
        node_destroy(rec->f);
        node_destroy(rec->g);
        break;
    }
    case NODE_SEARCH:
    {
        struct node_search *search;
        search = (struct node_search *) n->data;
        node_destroy(search->p);
        break;
    }
    case NODE_ZERO:
    case NODE_PROJECTION:
    case NODE_SUCCESSOR:
        break;
    }
    free(n->data);
    free(n);
}

/*!
    Allocates a new node array with \a e elements.
 */
struct node **
node_array_new(size_t e)
{
    return calloc(e, sizeof(struct node));
}

/*!
    Returns the result of the computation described by the provided node tree.
 */
int
compute(const struct node *n, int *x, size_t args)
{
    switch (n->type)
    {
    case NODE_ZERO:
        return 0;
    case NODE_PROJECTION:
    {
        struct node_projection *proj;
        proj = (struct node_projection *) n->data;
        return x[proj->place];
    }
    case NODE_SUCCESSOR:
        return (*x) + 1;
    case NODE_COMPOSITION:
    {
        struct node_composition *comp;
        struct node **curr;
        int j;
        comp = (struct node_composition *) n->data;
        curr = comp->g;
        int y[comp->places];
        j = 0;
        while (j < comp->places) {
            y[j++] = compute(*curr, x, args);
            ++curr;
        }
        return compute(comp->f, y, j);
    }
    case NODE_RECURSION:
    {
        struct node_recursion *rec;
        rec = (struct node_recursion *) n->data;
        if (0 == x[args - 1]) {
            return compute(rec->f, x, args - 1);
        } else {
            int nx[args + 1];
            --x[args - 1];
            nx[0] = compute(n, x, args);
            memcpy(&nx[1], x, args * sizeof(int));
            ++x[args - 1];
            return compute(rec->g, nx, args + 1);
        }
    }
    case NODE_SEARCH:
    {
        struct node_search *search;
        search = (struct node_search *) n->data;
        int i, lim;

        lim = x[args - 1];
        for (i = 0; i < lim; ++i) {
            x[args - 1] = i;
            if (1 == compute(search->p, x, args))
                return i;
        }
        return lim;
    }
    } /* end switch */

    /*
     * We should never reach here!
     */
    assert(0);
    return -1;
}
