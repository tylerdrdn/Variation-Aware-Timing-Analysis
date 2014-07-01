#ifndef NETPARSER_H_
#define NETPARSER_H_

#include "hashTable.h"
#include "types.h"

extern csrt *ratCsrt;
extern int numOfCsrt;

int netParser(char *filename, hashtable *piHash, hashtable *poHash, hashtable *intrnlHash, clk *clk, unsigned int numOfSims);

void makeUpNet(struct _hashTable *intrnlHash, struct _hashTable *poHash, struct _hashTable *piHash, res *resLS, cap *capLS, queue *auxQ, node *netStrtPnt);

#endif /* NETPARSER_H_ */
 
