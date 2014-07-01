#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libParse.h"

#define LINE 768
#define NOFMETALS 2
#define DEFAULT_SIZE 9
#define NOFMETALS 2

/* PROTOTYPES */

/**
 * initCellLib
 *
 * Initializes the internal global Cell Library Dictionary structure with the first level
 * vector allocated with size denoted by estim parameter.
 * The whole structure lies on dynamic memory handling for flexibility purposes.
 * When given the default value 0 as an argument the first level vector is allocated
 * according to a defined default size.
 *
 * @param estim	-Size of the first level vector
 * @return void
 */
void initCellLib(int estim);


libNode *allocateLevel(int level);


/**
 * classifyCell
 *
 * Classifies the read cell <key> into the Cell Library Dictionary.
 * Returns the proper position of the Dictionary that the cell structure should be stored.
 *
 * @param key	-The name of the cell
 * @return pos	-The memory position of the Dictionary the cell should be stored.
 */
cell *classifyCell(char *key);


/**
 * searchLevel
 *
 * Find the index of the desired key value in the libNode structure of level level.
 *
 * @param node		-Dictionary like structure with keys and corresponding objects
 * @param keyVal 	-Key value to search
 * @param level		-Level of structure to be searched
 * @return position	-Final returned position. If key value is not found, -1 is returned.
 */
int searchLevel(libNode *node, char *keyVal, int level);


/**
 * expandLevel
 *
 * Expands the capacity of the libNode structure of level level.
 *
 * @param vec				-libNode to be resized
 * @param level 			-Level of structure to be resized
 * @return final_position	-The actual index of the new last entry
 */
int expandLevel(libNode *vec, int level);


/**
 * readSlewDelay
 *
 * Reads the fall and rise slew values and fall and rise delay values in line buf.
 * The values are stored in slewVlas and delayVals respectively.
 *
 * @param buf 		-Parsed Line containing slew and delay information
 * @param slewVals	-fall and rise slew info structures
 * @param delayVals	-fall and rise delay info structures
 * @return void
 */
void readSlewDelay(char *buf, slewDef *slewVals[], delay *delayVals[]);


/**
 * readFallRiseConst
 *
 * Reads the fall and rise constraints information contained in line buf.
 * The values are stored in the fConst and rConst structs respectively.
 *
 * @param buf		-Parsed Line containing slew and delay information
 * @param fConst	-Fall Constraints Info struct
 * @param rConst	-Rise Constraints Info struct
 * @return void		-
 */
void readFallRiseConst(char *buf, fallConstraint *fConst, riseConstraint *rConst);

#define NOFMETALS 2

/****   Metal Related Variables   ****/
int sigma[NOFMETALS];
double resScaleFactor[NOFMETALS];
double capScaleFactor[NOFMETALS];

libNode *cellLib;

int parseLib(char *fileName) {
	FILE *libFile = NULL;

	/****      General Variables      ****/
	int i;
	char lineBuf[LINE];
	char *token;
	cell *cellLeaf;
	char *pinName;
	char *copy;

	initCellLib(9);

	libFile = fopen(fileName , "r");

	if(libFile == NULL) {
		printf("ERROR: Missing Library File / Wrong path to library file.\n");
		return 0;
	}

	//Read metal Information
	for(i = 0; i < NOFMETALS; ++i) {
		fscanf(libFile, "%255s", lineBuf);																// read metal - dummy string
		fscanf(libFile, "%d", &sigma[i]);
		fscanf(libFile, "%lf", &resScaleFactor[i]);
		fscanf(libFile, "%lf", &capScaleFactor[i]);
	}

	/*Read Cell Information*/

	while(fgets( lineBuf, LINE, libFile )) {
		//printf("\n%s\n",lineBuf);
		copy = strdup(lineBuf);

		token = strtok(lineBuf, " :\t\n");

		if(token != NULL) {
			if(!strcmp(token, "cell")) {
				token = strtok(NULL, " :\t\n");
				//printf ("Found Cell Name : %s\n", token);
				cellLeaf = classifyCell(token);

				free(copy);
				continue;
			}

			if(!strcmp(token, "pin")) {
				token = strtok(NULL, " :\t\n");
				//printf("Pin Name : %s\n", token);

				pinName = strdup(token);

				token = strtok(NULL, " :\t\n");
				//printf("Pin Type : %s\n", token);

				if(!strcmp(token, "input")) {
					//if (cellLeaf)
						//printf ( "Modifying cell : %s\n", cellLeaf->type);

					cellLeaf->inPinsCnt++;
					cellLeaf->inputPins = (inPins **)realloc(cellLeaf->inputPins, cellLeaf->inPinsCnt * sizeof(inPins *));

					cellLeaf->inputPins[cellLeaf->inPinsCnt - 1] = (inPins *)malloc(sizeof(inPins));

					cellLeaf->inputPins[cellLeaf->inPinsCnt - 1]->pin = pinName;
					//printf ( "Pin Name : %s\n", cellLeaf->inputPins[ cellLeaf->inPinsCnt - 1 ]->pin );

					token = strtok(NULL, " :\t\n");
					//printf("Fall Cap : %s\n", token);
					cellLeaf->inputPins[cellLeaf->inPinsCnt - 1]->fallCap = atof(token);

					token = strtok(NULL, " :\t\n");
					//printf("Rise Cap : %s\n", token);
					cellLeaf->inputPins[cellLeaf->inPinsCnt - 1]->riseCap = atof(token);
				}

				if(!strcmp(token, "output")) {
					//if (cellLeaf)
					//printf ( "Modifying cell : %s\n", cellLeaf->type);

					cellLeaf->outPinsCnt++;
					cellLeaf->outPins = (char **)realloc(cellLeaf->outPins, cellLeaf->outPinsCnt * sizeof(char *));

					cellLeaf->outPins[cellLeaf->outPinsCnt - 1] = strdup(pinName);
					//printf("Pin Name : %s\n", cellLeaf->outPins[cellLeaf->outPinsCnt - 1]);
				}

				if(!strcmp(token, "clock")) {
					cellLeaf->flipFlop = (ffStruct *)malloc(sizeof(ffStruct));
					cellLeaf->flipFlop->setup = NULL;
					cellLeaf->flipFlop->hold  = NULL;
					cellLeaf->flipFlop->setupCnt = 0;
					cellLeaf->flipFlop->holdCnt  = 0;


					//WARNING Limited Variable scope to this block ONLY
					ffClk *clock;

					clock = (ffClk *)malloc(sizeof(ffClk));
					clock->pin = strdup (pinName);

					token = strtok(NULL, " :\t\n");
					clock->fallCap = atof(token);
					//printf ("Fall Cap : %e\n", clock->fallCap);

					token = strtok(NULL, " :\t\n");
					clock->riseCap = atof(token);
					//printf ( "Rise Cap : %e\n", clock->riseCap );

					cellLeaf->flipFlop->clock = clock;
				}

				free(copy);
				continue;
			}

			if(!strcmp(token, "timing")) {
				//printf( "\n\n>In timing...\n\n");

				cellLeaf->timingsCnt++;
				cellLeaf->timing = (timing **)realloc(cellLeaf->timing, cellLeaf->timingsCnt * sizeof(timing *));
				cellLeaf->timing[cellLeaf->timingsCnt - 1] = (timing *)malloc(sizeof(timing));

				token = strtok(NULL, " :\t\n");
				cellLeaf->timing[cellLeaf->timingsCnt - 1]->inpPin = strdup(token);
				token = strtok(NULL, " :\t\n");
				cellLeaf->timing[cellLeaf->timingsCnt - 1]->outPin = strdup(token);

				token = strtok(NULL, " :\t\n");
				if(!strcmp(token, "positive_unate"))
					cellLeaf->timing[cellLeaf->timingsCnt - 1]->timingSense = POSITIVE_U;
				else if (!strcmp(token, "negative_unate"))
					cellLeaf->timing[cellLeaf->timingsCnt - 1]->timingSense = NEGATIVE_U;
				else
					cellLeaf->timing[cellLeaf->timingsCnt - 1]->timingSense = NON_U;

				//WARNING Limited Variable scope to this block ONLY
				slewDef *slewInf[2];
				delay *delayInf[2];

				readSlewDelay(copy, slewInf, delayInf);
				cellLeaf->timing[cellLeaf->timingsCnt - 1]->fallSlew = slewInf[0];
				cellLeaf->timing[cellLeaf->timingsCnt - 1]->riseSlew = slewInf[1];
				cellLeaf->timing[cellLeaf->timingsCnt - 1]->fallDelay = delayInf[0];
				cellLeaf->timing[cellLeaf->timingsCnt - 1]->riseDelay = delayInf[1];

				free(copy);
				continue;
			}

			if(!strcmp(token, "setup")) {
				setup *temp;

				cellLeaf->flipFlop->setupCnt++;
				cellLeaf->flipFlop->setup = (setup **)realloc(cellLeaf->flipFlop->setup, cellLeaf->flipFlop->setupCnt * sizeof(setup *));

				cellLeaf->flipFlop->setup[cellLeaf->flipFlop->setupCnt - 1] = (setup *)malloc(sizeof(setup));

				temp = cellLeaf->flipFlop->setup[ cellLeaf->flipFlop->setupCnt - 1 ];

				token = strtok(NULL, " :\t\n");
				temp->clockPin = strdup(token);

				token = strtok(NULL, " :\t\n");
				temp->inputPin = strdup(token);

				token = strtok(NULL, " :\t\n");

				if(!strcmp(token, "rising"))
					temp->edge = RISING;
				else
					temp->edge = FALLING;

				temp->fallConstr = (fallConstraint *)malloc(sizeof(fallConstraint));
				temp->riseConstr = (riseConstraint *)malloc(sizeof(riseConstraint));

				readFallRiseConst(copy, temp->fallConstr, temp->riseConstr);

				free(copy);
				continue;
			}

			if(!strcmp(token, "hold")) {
				hold *temp;

				cellLeaf->flipFlop->holdCnt++;
				cellLeaf->flipFlop->hold = (hold **)realloc(cellLeaf->flipFlop->hold, cellLeaf->flipFlop->holdCnt * sizeof(setup *));

				cellLeaf->flipFlop->hold[cellLeaf->flipFlop->holdCnt - 1] = (hold *)malloc(sizeof(hold));

				temp = cellLeaf->flipFlop->hold[cellLeaf->flipFlop->holdCnt - 1];

				token = strtok(NULL, " :\t\n");
				temp->clkPin = strdup(token);

				token = strtok(NULL, " :\t\n");
				temp->inpPin = strdup(token);

				token = strtok(NULL, " :\t\n");

				if(!strcmp(token, "rising"))
					temp->edge = RISING;
				else
					temp->edge = FALLING;

				temp->fallConstr = (fallConstraint *)malloc(sizeof(fallConstraint));
				temp->riseConstr = (riseConstraint *)malloc(sizeof(riseConstraint));

				readFallRiseConst(copy, temp->fallConstr, temp->riseConstr);

				free(copy);
				continue;
			}

			token = strtok(NULL, " :\t\n");
		}
	}

	fclose(libFile);

	return 0;
}



void readFallRiseConst(char *buf, fallConstraint *fConst, riseConstraint *rConst) {
	char *token;
	int i;

	token = strtok(buf, " :\t\n");
	for(i = 0; i < 3; ++i)
		token = strtok(NULL, " :\t\n");

	token = strtok(NULL, " :\t\n");
	fConst->g = atof(token);
	token = strtok(NULL, " :\t\n");
	fConst->h = atof(token);
	token = strtok(NULL, " :\t\n");
	fConst->j = atof(token);

	token = strtok(NULL, " :\t\n");
	rConst->m = atof(token);
	token = strtok(NULL, " :\t\n");
	rConst->n = atof(token);
	token = strtok(NULL, " :\t\n");
	rConst->p = atof(token);
}


void readSlewDelay(char *buf, slewDef *slewVals[], delay *delayVals[]) {
	char *token;
	int i;

	for(i = 0; i < 2; ++i) {
		slewVals[i] = (slewDef *)malloc(sizeof(slewDef));
		delayVals[i] = (delay *)malloc(sizeof(delay));
	}

	token = strtok(buf, " :\t\n");
	for(i = 0; i < 3; ++i)
		token = strtok(NULL, " :\t\n");

	for(i = 0; i < 2; ++i) {
		token = strtok(NULL, " :\t\n");
		slewVals[i]->x = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->y = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->z = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->ksv = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->kst = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->ksl = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->ksw = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->ksh = atof(token);
		token = strtok(NULL, " :\t\n");
		slewVals[i]->ksr = atof(token);
	}

	for(i = 0; i < 2; ++i) {
		token = strtok(NULL, " :\t\n");
		delayVals[i]->a = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->b = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->c = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->kdv = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->kdt = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->kdl = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->kdw = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->kdh = atof(token);
		token = strtok(NULL, " :\t\n");
		delayVals[i]->kdr = atof(token);
	}
}


void initCellLib(int estim) {
	cellLib = (libNode *)malloc(sizeof(libNode));

	if(!estim) {
		cellLib->keys = (char **)calloc(DEFAULT_SIZE, sizeof(char *));
		cellLib->objList = (libNode **)calloc(DEFAULT_SIZE, sizeof(libNode *));
		cellLib->leafCells = NULL;

		cellLib->level = 1;
		cellLib->size = DEFAULT_SIZE;

		//cellLib->accPattern[0] = -1;
		//cellLib->accPattern[1] = -1;
		//cellLib->accPattern[2] = -1;

		return;
	}

	cellLib->keys = (char **)calloc(estim, sizeof(char *));
	cellLib->objList = (libNode **)calloc(estim, sizeof(libNode *));
	cellLib->leafCells = NULL;

	cellLib->level = 1;
	cellLib->size = estim;

//	cellLib->accPattern[0] = -1;
//	cellLib->accPattern[1] = -1;
//	cellLib->accPattern[2] = -1;
	return;
}


libNode *allocateLevel(int level) {
	libNode *parent;

	parent = (libNode *)malloc(sizeof(libNode));

	if(level == 2) {
		parent->keys = (char **)calloc(1, sizeof(char *));
		parent->objList = (libNode **)calloc(1, sizeof(libNode *));
		parent->leafCells = NULL;

		parent->level = 2;
		parent->size = 1;

		return parent;
	}

	if(level == 3) {
		parent->keys = (char **)calloc(1, sizeof(char *));
		parent->objList = NULL;
		parent->leafCells = (cell **)calloc(1, sizeof(cell *));

		parent->level = 3;
		parent->size = 1;

		return parent;
	}
	return NULL;
}


int searchLevel(libNode *node, char *keyVal, int level) {
	int i;

	for(i = 0; i < (node->size); i++) {
		if(!strcmp(keyVal, node->keys[i]))
			return i;
	}
	return -1;
}


int expandLevel(libNode *vec, int level) {
	vec->size += 1;

	if(level == 2 || level == 1) {
		vec->keys = (char **)realloc(vec->keys, (vec->size) * sizeof (char *));
		vec->objList = (libNode **)realloc(vec->objList, (vec->size) * sizeof(libNode *));

		/* Initialize pointers */
		vec->keys[vec->size - 1] = NULL;
		vec->objList[vec->size - 1] = NULL;
	}

	if(level == 3) {
		vec->keys = (char **)realloc(vec->keys, (vec->size) * sizeof (char *));
		vec->leafCells = (cell **)realloc(vec->leafCells, (vec->size) * sizeof(cell *));

		/* Initialize pointers */
		vec->keys[vec->size - 1] = NULL;
		vec->leafCells[vec->size - 1] = NULL;
	}
	return(vec->size - 1);																				//return the actual index of the new last entry
}


cell *classifyCell(char *key) {

	int i;
	int posL1;
	int posL2;
	int posL3;
	char keyL1[2];
	char *keyL2;
	char *keyL3;
	char *buf;
	char *name;

	cell *interm;

	name = strdup(key);

	keyL1[0] = key[0];
	keyL1[1] = '\0';

	buf = strtok(name, "_");
	keyL2 = strdup(buf);

	buf = strtok(NULL, "_");
	keyL3 = strdup(buf);

	//printf ( "--- %s \n",keyL1 );
	//printf ( "--- %s \n",keyL2 );
	//printf ( "--- %s \n",keyL3 );

	//Level 1 Classifying

	posL1 = -1;

	if(!cellLib->keys[0]) {
		cellLib->keys[0] = (char *)malloc(sizeof(keyL1));
		strcpy(cellLib->keys[0], keyL1);
		posL1 = 0;
	}
	else {
		for(i = 0; i < cellLib->size; ++i) {
			if((cellLib->keys[i]) && (!strcmp(cellLib->keys[i], keyL1))) {								//!!WARNING: NULL pointer must be checked first
				posL1 = i;
				break;
			}

			if(!cellLib->keys[i]) {
				cellLib->keys[i] = (char *)malloc(sizeof(keyL1));
				strcpy(cellLib->keys[i], keyL1);
				posL1 = i;
				break;
			}
		}
	}

	if(posL1 == -1) {																					//expand Level 1 if not sufficiently initialized
		posL1 = expandLevel(cellLib, 1);
		cellLib->keys[posL1] = (char *)malloc(sizeof(keyL1));
		strcpy(cellLib->keys[posL1], keyL1);
	}

	//Level 2 Classification
	//printf ( "\t\t L1 position of pointer: %d\n",posL1);

	posL2 = -1;

	if(!cellLib->objList[posL1]) {
		cellLib->objList[posL1] = allocateLevel(2);
		cellLib->objList[posL1]->keys[0] = strdup(keyL2);
		posL2 = 0;
	}
	else {
		posL2 = searchLevel(cellLib->objList[posL1], keyL2, 2);

		if(posL2 < 0) {
			posL2 = expandLevel(cellLib->objList[posL1], 2);
			cellLib->objList[posL1]->keys[posL2] = strdup(keyL2);
		}
	}

	//Level 3 Classification
	//printf ( "\t\t L2 position of pointer: %d\n",posL2);

	posL3 = -1;

	if(!cellLib->objList[posL1]->objList[posL2]) {
		cellLib->objList[posL1]->objList[posL2] = allocateLevel(3);
		cellLib->objList[posL1]->objList[posL2]->keys[0] = strdup(keyL3);
		posL3 = 0;
	}
	else {
		posL3 = searchLevel(cellLib->objList[posL1]->objList[posL2], keyL3, 3);

		if(posL3 < 0) {
			posL3 = expandLevel(cellLib->objList[posL1]->objList[posL2], 3);
			cellLib->objList[posL1]->objList[posL2]->keys[posL3]  = strdup(keyL3);
		}
	}

	//printf ( "\t\t L3 position of pointer: %d\n",posL3);

	//cellLib->accPattern[0] = posL1;
	//cellLib->accPattern[1] = posL2;
	//cellLib->accPattern[2] = posL3;

	cellLib->objList[posL1]->objList[posL2]->leafCells[posL3] = (cell *)malloc(sizeof(cell));

	interm = cellLib->objList[posL1]->objList[posL2]->leafCells[posL3];

	interm->type = strdup(name);
	interm->flipFlop = NULL;
	interm->inputPins = NULL;
	interm->outPins = NULL;
	interm->timing = NULL;
	interm->inPinsCnt = 0;
	interm->outPinsCnt = 0;
	interm->timingsCnt = 0;

	free(name);

	return interm;
}


cell *findCell(libNode *libStruct, char *key, int *history) {
	int posL1;
	int posL2;
	int posL3;
	char keyL1[2];
	char *keyL2;
	char *keyL3;
	char *delimiter;
	char *name;

	name = strdup(key);

	keyL1[0] = name[0];
	keyL1[1] = '\0';

	keyL2 = name;

	delimiter = strstr(name, "_");
	*delimiter = '\0';

	keyL3 = delimiter + 1;

//	printf ( "--- %s \n",keyL1 );
//	printf ( "--- %s \n",keyL2 );
//	printf ( "--- %s \n",keyL3 );

	posL1 = -1;
	posL2 = -1;
	posL3 = -1;

	posL1 = searchLevel(libStruct, keyL1, 1);

	if(posL1 >= 0) {
		posL2 = searchLevel(libStruct->objList[posL1], keyL2, 2);

		if(posL2 >= 0) {
			posL3 = searchLevel(libStruct->objList[posL1]->objList[posL2], keyL3, 3);

			if(posL3 >= 0) {
				if(history) {
					history[0] = posL1;
					history[1] = posL2;
					history[2] = posL3;
				}

				free(name);
				return(libStruct->objList[posL1]->objList[posL2]->leafCells[posL3]);
			}
			else {
				free(name);
				return NULL;
			}
		}
		else {
			free(name);
			return NULL;
		}
	}
	else {
		free(name);
		return NULL;
	}
}
