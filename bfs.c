#include <stdio.h>
#include <stdlib.h>

#include "bfs.h"
#include "types.h"
#include "hashtable.h"
#include "queue.h"

void delayArc(hashTable *piHash) {

	int i;
	node *aux;
	node *adj;
	queue *auxQueue;
	state changed;

	/* Initialize a queue */

	auxQueue = (queue*) malloc(sizeof(queue));
	auxQueue->head = auxQ->tail = NULL;

	for(i = 0; i < piHash->size; i++) {
		aux = piHash->nodes[i];
		aux.visited = TRUE;

		enqueue(auxQueue, aux);
	}

	while(!isEmpty(auxQueue)) {																			/* While queue is not empty */
		aux = dequeue(auxQueue);

		if(aux->type == "inpin") {
			for(i = 0; i < aux->netEndPntCnt-1; i++) {													/* For all edges adjacent to aux node => for all the ending nodes which have as starting point aux node */
				adj = aux->linkedFw[i];																	/* adjacent contains the next of the nodes that follow aux node */
				changed = FALSE;

				//Co_r = h(fanout(u), RISE)																/* data from the interconnect */
																										/* calculation phase */
				//Co_f = h(fanout(u), FALL)

				if(adj->cellType->timing[cellType->timingsCnt-1]->timingSense == POSITIVE_U) {
					
					
					
					
				}
				else if(adj->cellType->timing[cellType->timingsCnt-1]->timingSense == NEGATIVE_U) {
					
					
					
					
				}
				else {																					/* NON UNATE */
					
					
					
					
					
				}

				adj->df = adj->cellType->timing->fallDelay;
				adj->dr = adj->cellType->timing->riseDelay;

				if(
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}