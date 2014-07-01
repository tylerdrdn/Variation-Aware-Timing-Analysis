#ifndef QUEUE_H_
#define QUEUE_H_

#include "types.h"

extern void enqueue(queue *Q, node *node);

extern node *dequeue(queue *Q);

extern int isEmpty(queue *Q);

#endif /* QUEUE_H_ */
