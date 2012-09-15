#include <malloc.h>
#include <assert.h>
#include "buf.h"

static void
bufput(struct buf *b, const void *data, size_t len)
{
    assert(b && b->unit);

    if (b->size + len > b->asize && buf_grow(b, b->size + len) < 0)
        return;

    memcpy(b->data + b->size, data, len);
    b->size += len;
}

struct buf *
buf_new(size_t unit_size)
{
    struct buf *newb;

    newb = malloc(sizeof(struct buf));
    if (newb) {
        newb->asize = 0;
        newb->size  = 0;
        newb->unit  = unit_size;
        newb->data  = NULL;
    }
    return newb;
}

void
buf_destroy(struct buf *b)
{
    if (!b)
        return;

    free(b->data);
    free(b);
}

buferror_t
buf_grow(struct buf *b, size_t n)
{
    size_t a;
    void *data;

    assert(b && b->unit);

    if (n > BUF_MAX_MEM_SIZE)
        return BUF_ENOMEM;
    if (b->asize >= n)
        return BUF_OK;
    a = b->asize + b->unit;
    while (a < n)
        a += b->unit;

    data = realloc(b->data, a);
    if (!data)
        return BUF_ENOMEM;
    b->data = data;
    b->asize = a;
    return BUF_OK;
}

void
buf_append_chars(struct buf *b, const char *s)
{
    bufput(b, s, strlen(s));
}

/*!
 *  Compares two buffers. Returns 1 if the two buffers have identical length
 *  and data.
 */
int
buf_compare(struct buf *b1, struct buf *b2)
{
    unsigned int i;
    char *d, *e;

    if (!b1 || !b2 || b1->size != b2->size)
        return 0;
    d = b1->data;
    e = b2->data;
    for (i = 0; i < b1->size; ++i)
        if (d[i] != e[i])
            return 0;
    return 1;
}
