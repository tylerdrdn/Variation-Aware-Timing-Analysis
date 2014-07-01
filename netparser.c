#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "hashTable.h"

#define LINESIZE 1024
#define NUM_OF_SIMS 1000


csrt *ratCsrt = NULL;
int numOfCsrt = 0;


/******************************************************************************************************************************************************************************************************/


int netParser(char *filename, hashTable *piHash, hashTable *poHash, hashTable *intrnlHash, clk *clk, unsigned int numOfSims) {

	FILE *fnet;
	char *token;
	char *nodeID;
	char lineBuf[LINESIZE];
	ratCsrt = (csrt*)malloc(sizeof(csrt));
	node *currNode = NULL;

	/* Auxiliary lists used to keep the cap and res of a net instance */

	res *resLS = NULL;
	cap *capLS = NULL;

	queue *auxQ;

	/* Initialize a queue */
	auxQ = (queue*) malloc(sizeof(queue));
	auxQ->head = auxQ->tail = NULL;

	fnet = fopen (filename, "r");

	if(fnet == NULL) {
		printf("Error. Unable to open the netlist input file.\n");
		return(-1);
	}

	node *netStrtPnt, *netEndPnt;
	unsigned char firstNetInstance = 1;																						// flag

	/*Read one line*/

	while(fgets(lineBuf, LINESIZE, fnet)) {
		token = strtok(lineBuf, " :\t\n");

		if(!strcmp(token, "input")) {
			token = strtok(NULL, " :\t\n");
			nodeID = strdup(token);

			currNode = insertHash(piHash, nodeID, 0);																		//place current node to hash table for primary inputs

			currNode->slew = (slew*) malloc(numOfSims * sizeof(slew));
			currNode->at = (at*) malloc(numOfSims * sizeof(at));

			unsigned int simIdx;
			for(simIdx = 0; simIdx < numOfSims; ++simIdx) {

				/* Default at values at primary inputs */
				currNode->at[simIdx].erlFall = currNode->at[simIdx].ltFall = 0.0;
				currNode->at[simIdx].erlRise = currNode->at[simIdx].ltRise = 0.0;

				/* Default slew values at primary inputs */
				currNode->slew[simIdx].erlFall = currNode->slew[simIdx].ltFall = 5e-12;
				currNode->slew[simIdx].erlRise = currNode->slew[simIdx].ltRise = 5e-12;
			}
		}
		else if(!strcmp(token, "output")) {
			token = strtok(NULL, " :\t\n");
			nodeID = strdup(token);

			currNode = insertHash(poHash, nodeID, 1);
		}
		else if(!strcmp(token, "instance")) {
			char *pin;
			char *type;

			token = strtok(NULL, " :\t\n");
			type = strdup(token);																							//the token is kept to be compared to the library cell types

			token = strtok(NULL, " :\t\n");
			while(token != NULL) {

				pin = strdup(token);

				if((*pin == "Q") || (*pin =="QN") || (*pin == "Z") || (*pin == "ZN")) {

					token = strtok(NULL, " :\t\n");
					nodeID = token;

					if(!(currNode = findHash(intrnlHash, nodeID)))															// if node doesn't exist
			    		if(!(currNode = findHash(poHash, nodeID)))
			    			currNode = insertHash(intrnlHash, strdup(nodeID), 0);
							currNode->type = "outpin";
							currNode->cellType = findCell(cellLib, type, NULL);
				}

				else {
					token = strtok ( NULL, " :\t\n" );
					nodeID = token;

					if (!(currNode = findHash(intrnlHash, nodeID)))															// if node doesn't exist
						if (!(currNode = findHash(piHash, nodeID)))
							currNode = insertHash(intrnlHash, strdup(nodeID), 0);
							currNode->type = "inpin";
							currNode->cellType = findCell(cellLib, type, NULL);
				}


				token = strtok(NULL, " :\t\n");
			}
		}

		else if(!strcmp(token, "slew")) {																					// only at primary inputs

			token = strtok(NULL, " :\t\n");

			nodeID = token;
			currNode = findHash(piHash, nodeID);

			double initFall, initRise;

			token = strtok(NULL, " :\t\n");
			initFall = atof(token);

			token = strtok(NULL, " :\t\n");
    		initRise = atof(token);

    		unsigned int simIdx;
			for(simIdx = 0; simIdx < numOfSims; ++simIdx) {
				currNode->slew[simIdx].erlRise = currNode->slew[simIdx].ltRise = initRise;
				currNode->slew[simIdx].erlFall = currNode->slew[simIdx].ltFall = initFall;
			}
		}

		else if(!strcmp(token, "at")) {																						// only at primary inputs

			double erlFall, ltFall, erlRise, ltRise;

			token = strtok( NULL, " :\t\n" );
			nodeID = token;

			currNode = findHash(piHash, nodeID);

			token = strtok(NULL, " :\t\n");
			erlFall = atof(token);

			token = strtok(NULL, " :\t\n");
			ltFall = atof(token);

			token = strtok(NULL, " :\t\n");
			erlRise = atof(token);

			token = strtok(NULL, " :\t\n");
			ltRise = atof(token);

			unsigned int simIdx;
			for(simIdx = 0; simIdx < numOfSims; ++simIdx) {
				currNode->slew[simIdx].erlRise = erlRise;
				currNode->slew[simIdx].ltRise = ltRise;
				currNode->slew[simIdx].erlFall = erlFall;
				currNode->slew[simIdx].ltFall = ltFall;
			}
		}

		else if(!strcmp(token, "clock")) {
			token = strtok(NULL, " :\t\n");
			nodeID = token;

			clk->clkNode = findHash(piHash, nodeID);

			token = strtok(NULL, " :\t\n");
    		clk->period = atof(token);																						// clock period
		}

		else if(!strcmp(token, "wire")) {
			if (!firstNetInstance) {
				makeUpNet(intrnlHash, poHash, piHash, resLS, capLS, auxQ, netStrtPnt); resLS = NULL; capLS = NULL;
			}

			firstNetInstance = 0;

			token = strtok(NULL, " :\t\n");
			nodeID = token;

			if(!(netStrtPnt = findHash(piHash, nodeID)))																	// It is sure that we will find this particular node
				netStrtPnt = findHash(intrnlHash, nodeID);

			token = strtok(NULL, " :\t\n");

			netStrtPnt->netEndPntCnt = 0;

			while(token != NULL) {
				nodeID = token;

				if(!(netEndPnt = findHash(poHash, nodeID)))																	// It is sure that we will find this particular node
					netEndPnt = findHash(intrnlHash, nodeID);

				netEndPnt->netStartNode = netStrtPnt;

				netStrtPnt->netEndPntCnt++;
				netStrtPnt->netEndNodes = (node**) realloc(netStrtPnt->netEndNodes, netStrtPnt->netEndPntCnt * sizeof(node*));

				netStrtPnt->netEndNodes[netStrtPnt->netEndPntCnt-1] = netEndPnt;

				token = strtok(NULL, " :\t\n");
			}
		}

		else if(!strcmp(token, "cap")) {
			cap *newCap = (cap*)malloc(sizeof(cap));

			token = strtok(NULL, " :\t\n");
			newCap->nodeID = strdup(token);

			token = strtok(NULL, " :\t\n");
			newCap->value = atof(token);

			newCap->nxt = capLS;
			capLS = newCap;
		}

		else if(!strcmp(token, "res")) {
			res *newRes = (res*)malloc(sizeof(res));

			token = strtok(NULL, " :\t\n");
			newRes->nodesID[0] = strdup(token);

			token = strtok(NULL, " :\t\n");
			newRes->nodesID[1] = strdup(token);

			token = strtok(NULL, " :\t\n");
			newRes->value = atof(token);

			newRes->nxt = resLS;
			resLS = newRes;
		}

		else if(!strcmp(token, "rat")) {
			token = strtok(NULL, " :\t\n");
			nodeID = token;

			currNode = findHash(poHash, nodeID);

			if(currNode == NULL)																							// it is sure that we will find the node here
				currNode = findHash(intrnlHash, nodeID);

			numOfCsrt++;

			if(numOfCsrt > 1)
				ratCsrt = (csrt*) realloc(ratCsrt, sizeof(csrt) * numOfCsrt);

			ratCsrt[numOfCsrt - 1].node = currNode;

			token = strtok(NULL, " :\t\n");
    		ratCsrt[numOfCsrt - 1].opMode = strdup(token);

			token = strtok(NULL, " :\t\n");
			ratCsrt[numOfCsrt - 1].ratFall = atof(token);

			token = strtok(NULL, " :\t\n");
			ratCsrt[numOfCsrt - 1].ratRise = atof(token);
		}
	}

	/* make up the last net */

	makeUpNet(intrnlHash, poHash, piHash, resLS, capLS,auxQ, netStrtPnt);
	resLS = NULL;
	capLS = NULL;

	free(auxQ);

	fclose(fnet);

	return 0;
}

/******************************************************************************************************************************************************************************************************/

void makeUpNet(hashTable *intrnlHash, hashTable *poHash, hashTable *piHash, res *resLS, cap *capLS, queue *auxQ, node *netStrtPnt) {
	node *currNode, *currNodeII;

	enqueue(auxQ, netStrtPnt);

	while((currNode = dequeue(auxQ)) != NULL) {
		double cap = findnRemCapLS(&capLS, currNode->id);

		currNode->cap = (cap == -1.0) ? 0 : cap;

		res *currRes, *prevRes;

		currRes = resLS;
		prevRes = NULL;

		while(currRes != NULL) {
			if (!strcmp(currNode->id, currRes->nodesID[0])) {
				if (!(currNodeII = findHash(intrnlHash, currRes->nodesID[1])))											// if node doesn't exist
					if(!(currNodeII = findHash(poHash, currRes->nodesID[1])))
						currNodeII = insertHash(intrnlHash, strdup(currRes->nodesID[1]), 0);

				enqueue(auxQ, currNodeII);

				currNodeII->linkedBw = currNode;
				currNodeII->res = currRes->value;

				currNode->len++;
				currNode->linkedFw = (node**) realloc(currNode->linkedFw, sizeof(node*) * currNode->len);
				currNode->linkedFw[currNode->len-1] = currNodeII;

				/* Delete a res node */

				if (prevRes == NULL) {
					resLS = currRes->nxt;

					free(currRes->nodesID[0]);
					free(currRes->nodesID[1]);
					free(currRes);

					currRes = resLS;
				}
				else {
					prevRes->nxt = currRes->nxt;

					free(currRes->nodesID[0]);
					free(currRes->nodesID[1]);
					free(currRes);

					currRes = prevRes->nxt;
				}
			}
			else if (!strcmp(currNode->id, currRes->nodesID[1])) {
				if (!(currNodeII = findHash(intrnlHash, currRes->nodesID[0])))											// if node doesn't exist
					if(!(currNodeII = findHash(poHash, currRes->nodesID[0])))
						currNodeII = insertHash(intrnlHash, strdup(currRes->nodesID[0]), 0);

				enqueue(auxQ, currNodeII);

				currNodeII->linkedBw = currNode;
				currNodeII->res = currRes->value;

				currNode->len++;
				currNode->linkedFw = (node**) realloc (currNode->linkedFw, sizeof(node*) * currNode->len);
				currNode->linkedFw[currNode->len - 1] = currNodeII;

				/* Delete a res node */

				if (prevRes == NULL) {
					resLS = currRes->nxt;

					free(currRes->nodesID[0]);
					free(currRes->nodesID[1]);
					free(currRes);

					currRes = resLS;
				}
				else {
					prevRes->nxt = currRes->nxt;

					free(currRes->nodesID[0]);
					free(currRes->nodesID[1]);
					free(currRes);

					currRes = prevRes->nxt;
				}
			}
			else {
				prevRes = currRes;
				currRes = currRes->nxt;
			}
		}
	}
}
