#ifndef COMP_SERIALIZE_H
#define COMP_SERIALIZE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "comp.h"
#include "buf.h"

typedef enum {
    SERIAL_DATA_OK      = 0,
    SERIAL_DATA_INVALID = -1
} ser_valid_t;

struct node *node_unserialize(struct buf *buf);
void node_serialize(struct node *node, struct buf* buf);
ser_valid_t node_serial_data_is_valid(struct buf *buf);

#ifdef __cplusplus
}
#endif

#endif /* COMP_SERIALIZE_H */

