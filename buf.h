#ifndef BUF_H
#define BUF_H

#ifdef __cplusplus
extern "C"
{
#endif

#define BUF_MAX_MEM_SIZE (32 * 1024 * 1024)

#include <string.h>

typedef enum {
    BUF_OK = 0,
    BUF_ENOMEM = -1
} buferror_t;

struct buf
{
    char *data;
    size_t size;
    size_t asize;
    size_t unit;
};

struct buf *buf_new(size_t unit_size);
void buf_destroy(struct buf *b);

buferror_t buf_grow(struct buf *b, size_t n);
void buf_append_chars(struct buf *b, const char *s);
void buf_nullterm(struct buf *b);

int buf_compare(struct buf *b1, struct buf *b2);

#ifdef __cplusplus
}
#endif

#endif /* BUF_H */
