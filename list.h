#ifndef LIST_H_
#define LIST_H_

extern void node_memory_deallocation(node *rootNode);

extern node *insertNodeLS(node *nodeLS, char *nodeID);

extern node *findNodeLS(node *nodeLS, char *nodeID);

extern void printLS(node *nodeLS);

extern double findnRemCapLS(cap **capLS, char *nodeID);

#endif /* LIST_H_ */
