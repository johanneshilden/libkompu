#ifndef COMP_H
#define COMP_H

#include <inttypes.h>
#include <stdlib.h>

enum node_type {
    NODE_ZERO = 0,
    NODE_PROJECTION,
    NODE_SUCCESSOR,
    NODE_COMPOSITION,
    NODE_RECURSION,
    NODE_SEARCH
};

struct node
{
    uint8_t type;
    void *data;
};

struct node_projection
{
    int place;
};

struct node_composition
{
    struct node *f;
    struct node **g;
    int places;
};

struct node_recursion
{
    struct node *f;
    struct node *g;
};

struct node_search
{
    struct node *p;
};

struct node *projection_node_new(int place);
struct node *zero_node_new();
struct node *successor_node_new();
struct node *composition_node_new(struct node *f, struct node **g);
struct node *recursion_node_new(struct node *f, struct node *g);
struct node *search_node_new(struct node *p);

void node_destroy(struct node *n);

struct node **node_array_new(size_t e);

int compute(const struct node *n, int *x, size_t args);

#endif /* COMP_H */
