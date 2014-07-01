#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"


node* findNodeLS(node *nodeLS, char *nodeID) {
	node *currNode = NULL;

	for(currNode = nodeLS->next; currNode != NULL; currNode = currNode->next)
		if(!strcmp(currNode->id, nodeID))
			return currNode;

	return NULL;
}


void printLS(node *nodeLS) {
	node *currNode;

	for(currNode = nodeLS->next; currNode != NULL; currNode = currNode->next)
		printf("node's id: %s\n", currNode->id);
}


void node_memory_deallocation(node *rootNode) {
	node *currNode = NULL, *prevNode = NULL;
	unsigned int i;

	for(prevNode = rootNode, currNode = rootNode->next; currNode != NULL; prevNode = currNode, currNode = currNode->next) {
		free(currNode->id);

		if(currNode->linkedFw != NULL)
			free(currNode->linkedFw);

		if(currNode->inpToCell != NULL)
			free(currNode->inpToCell);

		if(currNode->slew != NULL) {
			free(currNode->slew);

			if(currNode->netStartNode != NULL) {
				node *netStartNode = currNode->netStartNode;

				netStartNode->slew = NULL;

				for(i = 0; i < netStartNode->netEndPntCnt; ++i)
					netStartNode->netEndNodes[i]->slew = NULL;
			}

			for(i = 0; i < currNode->netEndPntCnt; ++i)
				currNode->netEndNodes[i]->slew = NULL;
		}

		if(currNode->at != NULL) {
			free(currNode->at);

			if(currNode->netStartNode != NULL) {
				node *netStartNode = currNode->netStartNode;

				netStartNode->at = NULL;

				for(i = 0; i < netStartNode->netEndPntCnt; ++i)
					netStartNode->netEndNodes[i]->at = NULL;
			}

			for(i = 0; i < currNode->netEndPntCnt; ++i)
				currNode->netEndNodes[i]->at = NULL;
		}

		if(currNode->netEndNodes != NULL)
			free(currNode->netEndNodes);

		free(prevNode);
	}

	free(prevNode);
}


node *insertNodeLS(node *nodeLS, char *nodeID) {
	node *newNode = NULL;

	newNode = (node*)malloc(sizeof(node));
	newNode->next = nodeLS->next;
	nodeLS->next = newNode;

	/* Node Initialization */

	newNode->id= nodeID;

	newNode->linkedFw = NULL;
	newNode->res = 0;
	newNode->len = 0;

	newNode->linkedBw = NULL;

	newNode->cap = 0;
	newNode->slew = NULL;																				// ( slew* )malloc( sizeof( slew ) );
    newNode->at= NULL;																					// ( at* )malloc( sizeof( at ) );

	newNode->inpToCell = NULL;
	newNode->numCellsAsInput = 0;

	newNode->outOfCell = NULL;

	newNode->netEndPntCnt = 0;
	newNode->netEndNodes = NULL;
	newNode->netStartNode = NULL;

	return newNode;
}


double findnRemCapLS(cap **capLS, char *nodeID) {
	cap *currCap;
	cap *prevCap;
	double cap = -1.0;

	for(prevCap = NULL, currCap = (*capLS); ((currCap != NULL) && (strcmp(currCap->nodeID, nodeID))); prevCap = currCap, currCap = currCap->nxt);

	if(currCap != NULL) {
		cap = currCap->value;

		if(prevCap == NULL)
			(*capLS) = currCap->nxt;
		else
			prevCap->nxt = currCap->nxt;

		free(currCap->nodeID);
		free(currCap);
	}

	return(cap);
}
