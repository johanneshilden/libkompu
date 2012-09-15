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
 *  \struct node
 *
 *  \brief The base node struct, holding a pointer to node specific data.
 */

/*!
 *  \struct node_projection
 *
 *  \brief A leaf projection node.
 */

/*!
 *  \struct node_composition
 *
 *  \brief Node representing an application of composition.
 */

/*!
 *  \struct node_recursion
 *
 *  \brief Node representing an application of primitive recursion.
 */

/*!
 *  \struct node_search
 *
 *  \brief Search operator node.
 */

/*!
 *  Creates a new projection function node.
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
 *  Creates a new zero function node.
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
 *  Creates a new successor function node.
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
 *  Creates a new composition node.
 *
 *  Explicit transformation permits scrambling variables, repeating variables,
 *  omitting variables, and substituting constants.
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
 *  Creates a new recursion node.
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
 *  Creates a new search node (the μ-operator).
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

struct node *
node_clone(struct node *n)
{
    struct node **g;
    union node_d_ptr d_ptr;
    int i;

    if (n) {
        switch (n->type)
        {
        case NODE_COMPOSITION:
            d_ptr.comp = (struct node_composition *) n->data;
            g = node_array_new(d_ptr.comp->places);
            for (i = 0; i < d_ptr.comp->places; ++i)
                g[i] = node_clone(d_ptr.comp->g[i]);
            return composition_node_new(node_clone(d_ptr.comp->f), g);
        case NODE_RECURSION:
            d_ptr.rec = (struct node_recursion *) n->data;
            return recursion_node_new(node_clone(d_ptr.rec->f),
                                      node_clone(d_ptr.rec->g));
        case NODE_SEARCH:
            d_ptr.search = (struct node_search *) n->data;
            return search_node_new(d_ptr.search->p);
        case NODE_PROJECTION:
            d_ptr.proj = (struct node_projection *) n->data;
            return projection_node_new(d_ptr.proj->place);
        case NODE_ZERO:
            return zero_node_new();
        case NODE_SUCCESSOR:
            return successor_node_new();
        } /* end switch */
    }
    return NULL;
}

/*!
 *  Destroys the provided node and releases associated memory.
 */
void
node_destroy(struct node *n)
{
    int i;
    union node_d_ptr d_ptr;
    struct node **curr;

    assert(n);

    switch (n->type)
    {
    case NODE_COMPOSITION:
        d_ptr.comp = (struct node_composition *) n->data;
        curr = d_ptr.comp->g;
        i = d_ptr.comp->places;
        while (i--) {
            node_destroy(*curr);
            ++curr;
        }
        free(d_ptr.comp->g);
        node_destroy(d_ptr.comp->f);
        break;
    case NODE_RECURSION:
        d_ptr.rec = (struct node_recursion *) n->data;
        node_destroy(d_ptr.rec->f);
        node_destroy(d_ptr.rec->g);
        break;
    case NODE_SEARCH:
        d_ptr.search = (struct node_search *) n->data;
        node_destroy(d_ptr.search->p);
        break;
    case NODE_ZERO:
    case NODE_PROJECTION:
    case NODE_SUCCESSOR:
        break;
    }
    free(n->data);
    free(n);
}

/*!
 *  Allocates a new node array with \a e elements.
 */
struct node **
node_array_new(size_t e)
{
    return calloc(e, sizeof(struct node));
}

/*!
 *  Returns the result of the computation described by the provided node tree.
 */
int
node_compute(const struct node *n, int *x, size_t args)
{
    union node_d_ptr d_ptr;
    struct node **curr;
    int i, j, lim;

    switch (n->type)
    {
    case NODE_ZERO:
        return 0;
    case NODE_PROJECTION:
        d_ptr.proj = (struct node_projection *) n->data;
        return x[d_ptr.proj->place];
    case NODE_SUCCESSOR:
        return (*x) + 1;
    case NODE_COMPOSITION:
    {
        d_ptr.comp = (struct node_composition *) n->data;
        curr = d_ptr.comp->g;
        int y[d_ptr.comp->places];
        j = 0;
        while (j < d_ptr.comp->places) {
            y[j++] = node_compute(*curr, x, args);
            ++curr;
        }
        return node_compute(d_ptr.comp->f, y, j);
    }
    case NODE_RECURSION:
        d_ptr.rec = (struct node_recursion *) n->data;
        if (0 == x[args - 1]) {
            return node_compute(d_ptr.rec->f, x, args - 1);
        } else {
            int nx[args + 1];
            --x[args - 1];
            nx[0] = node_compute(n, x, args);
            memcpy(&nx[1], x, args * sizeof(int));
            ++x[args - 1];
            return node_compute(d_ptr.rec->g, nx, args + 1);
        }
    case NODE_SEARCH:
        d_ptr.search = (struct node_search *) n->data;

        lim = x[args - 1];
        for (i = 0; i < lim; ++i) {
            x[args - 1] = i;
            if (1 == node_compute(d_ptr.search->p, x, args))
                return i;
        }
        return lim;
    } /* end switch */

    /*
     * We should never reach here!
     */
    assert(0);
    return -1;
}
