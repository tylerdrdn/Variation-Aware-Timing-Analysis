#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "list.h"

#define STRSIZE 40
#define SIZE 2048


/********************************************************************************
 *                                                                              *
 *                       DATA STRUCTURES DEFINITIONS                            *
 *                                                                              *
 ********************************************************************************/

struct _hashTable
{
	int size;
	node *root;
	node **nodes;
};

typedef struct _hashTable hashTable;

/********************************************************************************
 *                                                                              *
 *                            FUNCTION DECLARATIONS                             *
 *                                                                              *
 ********************************************************************************/


/**
 *  Function that creates a hashtable.
 *  @param size size of the hashtable.
 *  @return the hashtable.
 */
hashTable *createHash(int size);


/**
 *  Function to destroy the hashtable.
 *  @param hashtbl the hashtable.
 */
void destroyHash (hashTable *hashtbl);


/**
 *  Function to insert an element in hashtable.
 *  @param hashtable the hashtable.
 *  @param key the node we want to insert.
 *  @result a pointer to the the newNode has just added if successful and NULL in case of failure.
 */
node *insertHash(hashTable *hashtable, char *key, int sorted);


/**
 *  Function to find an element and return the position.
 *  @param hastbl the hashtable.
 *  @param key the name we search for.
 *  @return a pointer to the node if it is found or NULL in case it isn't found.
 */
node *findHash(hashTable *hashtbl, char *key);


/**
 *  Hash Function
 *  @param s string to be hashed.
 *  @return the position in the hash table.
 */
int hashFunc(char *s);


/**
 *  Function to print all the elements of the hashtable.
 *  @param hashtbl the hashtable.
 */
void printHash (hashTable *hashtbl);

#endif /* HASHTABLE_H_ */
 
