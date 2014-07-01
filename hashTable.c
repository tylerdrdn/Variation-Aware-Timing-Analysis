#include "hashTable.h"

hashTable *createHash(int size) {
	hashTable *hashtbl;
	unsigned int n;

	if(!(hashtbl = malloc(sizeof(hashTable)))) {
		return NULL;
	}
	if(!(hashtbl->nodes = calloc(size, sizeof(node)))) {
		free(hashtbl);
		return NULL;
	}

	for(n = 0; n < size; n++) {
		hashtbl->nodes[n] = (node*)malloc(sizeof(node));
		hashtbl->nodes[n]->next = NULL;
	}

	hashtbl->root = NULL;

	hashtbl->size = size;

	return(hashtbl);
}


void destroyHash(hashTable *hashtbl) {
	int n;
	node *rootNode;

	for(n = 0; n < hashtbl->size; n++) {
		rootNode = hashtbl->nodes[n];
		node_memory_deallocation(rootNode);
	}
	free(hashtbl->nodes);
	free(hashtbl);
}


node *insertHash(hashTable *hashtbl, char *key, int sorted) {
	node *currNode, *newNode;
	int hash;

	hash = hashFunc(key) % hashtbl->size;
	currNode = hashtbl->nodes[hash]->next;

	while(currNode) {
		if(!strcmp(currNode->id, key)) {
			//printf("Node Already Exists ... \n");
			return NULL;
		}
		currNode = currNode->next;
	}

	newNode = insertNodeLS(hashtbl->nodes[hash], key);

	/* Keep all nodes of a hash table in a list */

	if(sorted) { 																						/* Sorted list */
		node *currNode, *prevNode;

		for(prevNode = NULL, currNode = hashtbl->root; ((currNode != NULL) && (strcmp(newNode->id, currNode->id) > 0)); prevNode = currNode, currNode = currNode->x_nxt);

		newNode->x_nxt = currNode;

		if(prevNode != NULL)
			prevNode->x_nxt = newNode;
		else
			hashtbl->root = newNode;
	}
	else {																								/* Not sorted list */
		newNode->x_nxt = hashtbl->root;
		hashtbl->root = newNode;
	}

	return newNode;
}


node *findHash(hashTable *hashtbl, char *key) {
	node *currNode;
	int hash;

	hash = hashFunc(key) % hashtbl->size;
	currNode = hashtbl->nodes[hash]->next;

	while(currNode) {
		if(!strcmp(currNode->id, key))
			return currNode;

		currNode = currNode->next;
	}
	return NULL;
}


int hashFunc(char *s) {
	unsigned int tmp = 0;

	while (*s)
		tmp = tmp * 37 + *s++;

	return tmp%SIZE;
}


void printHash(hashTable *hashtbl) {
	int n;
	node *rootNode;

	for(n = 0; n < hashtbl->size; n++) {
		rootNode = hashtbl->nodes[n];
		printLS(rootNode);
	}
}
