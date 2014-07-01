#ifndef INTERCONNECT_H_
#define INTERCONNECT_H_

#include "types.h"
#include "GaussianOp.h"

/*****************************************************************
 *             			     Data Types				 			 *
 *****************************************************************/
typedef enum _corner
{
	NOMINAL = 0, SIGMA = 1
} corner;

/*
 * Path to tap stack
 */

typedef struct _pathnode													//Node of path
{
	node* mark;																//Circuit Node
	int indexfound;															//If node mark's parent has many children, this holds the index of the branch found.
	double resSum;															//Total path resistance till mark

	struct _pathnode *prev;
	struct _pathnode *nxt;
} pathnode;

typedef struct _path														//Double pointed stack
{
	pathnode *base;
	pathnode *top;
} path;

typedef struct _interInfo													//Interconnection delay & slew characteristics
{
	node *netStart;															//port of the net ( as ID )
	node **netEnd;															//taps of the net (the order of the taps is the same on the below matrices respectively)
	int nofTaps;

	gaussianDist **delay;													//delays under variation, ordered as denoted by netEndNodes indexing
	gaussianDist **slew;													//slews under variation, 4 values for every pin indexed as above
} interInfo;


/*****************************************************************
 *             			     Functions				 			 *
 *****************************************************************/


/*
 * Path Utility Functions
 */
path * initpath ();
void emptyPath (path *nodePath);

/*
 *	Path Delay Calculation
 */
void elmorePath(path *pathTap, node *pin, char *tapId, int corner, double delay[]);

/*
 *	Interconnect Characteristics Calculation (Delay / Slew)
 */
interInfo *interconnectCalc(node *pin);

#endif /* INTERCONNECT_H_ */
