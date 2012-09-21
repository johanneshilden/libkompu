#include <assert.h>
#include <stdio.h>
#include "comp_serialize.h"

static int
skip(char c)
{
    switch (c)
    {
    case '>':
    case '}':
    case ')':
    case ',':
    case ']':
        return 1;
    default:
        break;
    } /* end switch */
    return 0;
}

static struct node *unserialize(struct buf *buf, int *pos);

static struct node *
parse(struct buf *buf, int *pos)
{
    struct node *f;
    f = unserialize(buf, pos);
    while (skip(buf->data[*pos]))
        ++(*pos);
    return f;
}

static struct node **
parse_array(struct buf *buf, int *pos, int n)
{
    int i;
    struct node **g;
    g = node_array_new(n);
    for (i = 0; i < n - 1; ++i)
        g[i] = parse(buf, pos);
    g[n - 1] = NULL;
    return g;
}

static struct node *
unserialize(struct buf *buf, int *pos)
{
    static char str[20];
    char x, *bufdata;
    struct node *f;
    int i, n;
    assert(buf);

    x = buf->data[*pos];
    ++(*pos);
    switch (x)
    {
    case '0':
        return zero_node_new();
    case '{':
        n = 0;
        while ('}' != buf->data[*pos]) {
            str[n++] = buf->data[*pos];
            ++(*pos);
        }
        str[n] = '\0';
        return projection_node_new(atoi(str));
    case '+':
        return successor_node_new();
    case '[':
        bufdata = &buf->data[*pos];
        n = i = 0;
        while (i >= 0) {
            switch (*bufdata)
            {
            case '>':
            case '}':
            case ')':
            case ']':
                --i;
                break;
            case '<':
            case '{':
            case '(':
            case '[':
                ++i;
                break;
            case ',':
                if (!i) ++n;
            default:
                break;
            } /* end switch */
            ++bufdata;
        }
        f = parse(buf, pos);
        return composition_node_new(f, parse_array(buf, pos, n + 1));
    case '<':
        f = parse(buf, pos);
        return recursion_node_new(f, parse(buf, pos));
    case '(':
        return search_node_new(unserialize(buf, pos));
    case 'X':
    default:
        break;
    }
    return invalid_node_new();
}

/*!
 *  Creates a node from the string stored in \a buf, according to the rules
 *  described under node_serialize().
 */
struct node *
node_unserialize(struct buf *buf)
{
    int n;
    n = 0;
    return unserialize(buf, &n);
}

/*!
 *  Serializes \a node to the provided char buffer according to the following
 *  simple rules:
 *
 *  Node type         -> Becomes    Comment
 *  ----------------------------------------------------------------------------
 *  ZERO node         -> 0
 *  PROJECTION node   -> {X}        where X is the projection coordinate
 *  SUCCESSOR node    -> +          (just a single '+' character)
 *  COMPOSITION node  -> [?,?,?]    where ? is replaced with the node "legs"
 *  RECURSION node    -> <?,?>      where ? is replaced with the node "legs"
 *  SEARCH node       -> (?)        where ? is replaced with the sub-node
 *  INVALID node      -> X          (just a single 'X' character)
 */
void
node_serialize(struct node *node, struct buf* buf)
{
    struct node **g;
    union node_d_ptr d_ptr;
    static char str[20];

    assert(node && buf);

    switch (node->type)
    {
    case NODE_ZERO:
        buf_append_chars(buf, "0");
        break;
    case NODE_PROJECTION:
        d_ptr.proj = (struct node_projection *) node->data;
        buf_append_chars(buf, "{");
        snprintf(str, 20, "%i", d_ptr.proj->place);
        buf_append_chars(buf, str);
        buf_append_chars(buf, "}");
        break;
    case NODE_SUCCESSOR:
        buf_append_chars(buf, "+");
        break;
    case NODE_COMPOSITION:
        d_ptr.comp = (struct node_composition *) node->data;
        buf_append_chars(buf, "[");
        node_serialize(d_ptr.comp->f, buf);
        g = d_ptr.comp->g;
        while (*g) {
            buf_append_chars(buf, ",");
            node_serialize(*g, buf);
            ++g;
        }
        buf_append_chars(buf, "]");
        break;
    case NODE_RECURSION:
        d_ptr.rec = (struct node_recursion *) node->data;
        buf_append_chars(buf, "<");
        node_serialize(d_ptr.rec->f, buf);
        buf_append_chars(buf, ",");
        node_serialize(d_ptr.rec->g, buf);
        buf_append_chars(buf, ">");
        break;
    case NODE_SEARCH:
        d_ptr.search = (struct node_search *) node->data;
        buf_append_chars(buf, "(");
        node_serialize(d_ptr.search->p, buf);
        buf_append_chars(buf, ")");
        break;
    case NODE_INVALID:
    default:
        buf_append_chars(buf, "X");
        break;
    }
}
