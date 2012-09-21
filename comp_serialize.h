#ifndef COMP_SERIALIZE_H
#define COMP_SERIALIZE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "comp.h"
#include "buf.h"

struct node *node_unserialize(struct buf *buf);
void node_serialize(struct node *node, struct buf* buf);

#ifdef __cplusplus
}
#endif

#endif /* COMP_SERIALIZE_H */

