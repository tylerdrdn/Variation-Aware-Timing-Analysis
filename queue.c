#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

void enqueue(queue *Q, node *node) {
	qnode *newQnode;

	newQnode = (qnode*) malloc(sizeof(qnode));

	newQnode->node = node;

	if(Q->head == NULL)
		Q->head = newQnode;
	else
		Q->tail->nxt = newQnode;

	Q->tail = newQnode;
}

node* dequeue(queue *Q) {
	qnode *head;
	node *node;

	head = Q->head;

	if(head != NULL) {
		node = Q->head->node;

		if(Q->head == Q->tail)
			Q->head = Q->tail = NULL;
		else
			Q->head = Q->head->nxt;

		free(head);

		return(node);
	}
	else
		return NULL;
}

int isEmpty(queue *Q) {
	if(Q->head == NULL)
		return(1);
	else
		return(0);
}
