#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "interconnect.h"

//Defines

extern double capScaleFactor[2];
extern double resScaleFactor[2];

#define INTERC_DBG 0

/*
 * Internal prototypes
 */

interInfo *initInterInfo (node *pin, int len);
void pushPath (path *totap, node *markin, int index, double res);
node *popPath (path *totap);
void printPath (path *pathTap);

int pathFind (path *pathTap, node *root, double res, char *tapId);
void sumCaps (node *root, int reset, int corner, int betaCalc, double capsum[]);
void findPinCap (node *toPin, double *cap);

void treeDelay (interInfo *info);

double *betaCalc (path *pathTap, node *pin, char *tapId);

void printInfo (interInfo *nfo, int all);

void commitInfo (interInfo *nfo);

void treeSlew (interInfo *info);

/*********************
 * Utility functions *
 *********************/

interInfo *initInterInfo ( node *pin, int len )
{
	interInfo *temp;
	int i;

	temp = (interInfo *) malloc ( sizeof(interInfo) );

	//allocate delay - slew tables
	temp->delay = (gaussianDist **) malloc ( len * sizeof(gaussianDist *) );
	temp->slew = (gaussianDist **) malloc ( len * sizeof(gaussianDist *) );
	temp->netEnd = (node **) malloc ( len * sizeof(node *) );

	for (i = 0; i < len; ++i)
	{
		temp->delay[i] = (gaussianDist *) malloc ( 2 * sizeof(gaussianDist) );
		temp->slew[i] = (gaussianDist *) malloc ( 4 * sizeof(gaussianDist) );
	}

	//assign values
	for (i = 0; i < len; ++i)
		temp->netEnd[i] = pin->netEndNodes[i];

	temp->netStart = pin;
	temp->nofTaps = len;

	return temp;
}

path * initpath ()
{
	path *totap;

	totap = (path *) malloc ( sizeof(path) );

	totap->base = NULL;
	totap->top = NULL;

	return totap;
}

void pushPath ( path *totap, node *markin, int index, double res )
{
	pathnode *pnode;

	pnode = (pathnode *) malloc ( sizeof(pathnode) );

	pnode->mark = markin;
	pnode->indexfound = index;
	pnode->resSum = res;

	if (totap->base == NULL || totap->top == NULL )
	{
		totap->base = pnode;
		totap->top = pnode;
		pnode->prev = NULL;
		pnode->nxt = NULL;

		return;
	}

	pnode->nxt = NULL;
	totap->top->nxt = pnode;
	pnode->prev = totap->top;
	totap->top = pnode;
}

node *popPath ( path *totap )
{
	pathnode *pnode;
	node *temp;

	temp = totap->top->mark;
	pnode = totap->top;

	totap->top = pnode->prev;
	totap->top->nxt = NULL;
	free ( pnode );

	return temp;
}

void emptyPath ( path *nodePath )
{
	pathnode *cur;  // *temp;

	while (nodePath->top != NULL )
	{
		cur = nodePath->top;
		nodePath->top = cur->prev;
		free ( cur );
	}
}

void printPath ( path *pathTap )
{
	pathnode *cur;

	printf ( "#path input:%s\n", pathTap->top->mark->id );
	printf ( "#path tap:%s\n", pathTap->base->mark->id );

	for (cur = pathTap->top; cur != NULL ; cur = cur->prev)
	{
		printf ( "#%s", cur->mark->id );
		printf ( "\t index:%d", cur->indexfound );
		printf ( "\t res sum:%f\n", cur->resSum );
	}
}

void findPinCap ( node *toPin, double cap[] )
{

	cellInst *netCell;
	cell *libCell;

	char *pinName;

	if (toPin->inpToCell == NULL )
	{
		cap[0] = cap[1] = 0.0;
		return;
	}

	netCell = toPin->inpToCell[0];

	int j;

	for (j = 0; j < netCell->numOfInputs; j++)
	{
		if (!strcmp ( toPin->id, netCell->inPinToNode[j]->id ))
			pinName = netCell->inPins[j];
	}

	//libCell = findCell ( cellLib, netCell->type, NULL );
	libCell = netCell->cellType;

	for (j = 0; j < libCell->inPinsCnt; j++)
	{
		if (!strcmp ( libCell->inputPins[j]->pin, pinName ))
		{
			cap[0] = libCell->inputPins[j]->fallCap;  //Is always passed like this
			cap[1] = libCell->inputPins[j]->riseCap;
		}

	}
}

int pathFind ( path *pathTap, node *root, double res, char *tapId )
{
	int i;
	int onPath;
	double totalRes;

	totalRes = res;

	if (root)
	{
		totalRes += root->res;

		for (i = 0; i < root->len; i++)
		{
			onPath = pathFind ( pathTap, root->linkedFw[i], totalRes, tapId );

			if (onPath)
			{
				pushPath ( pathTap, root->linkedFw[i], i, totalRes + root->linkedFw[i]->res );
				return 1;
			}
		}

		if (root->len == 0)
		{
			//printf ( "$$%s\n", root->id );

			if (!strcmp ( root->id, tapId ))
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}

		return 0;
	}

	return 0;
}

void sumCaps ( node *root, int reset, int corner, int betaCalc, double capsum[] )
{
	int i;
	double interm[2];

	if (reset)
		capsum[0] = capsum[1] = 0.0;

	if (root)
	{
		double temp;

		if (corner)		//Sigma Case
		{
			if (root->len != 0)
			{
				if (!betaCalc)
				{
					temp = capsum[0] + root->cap * capScaleFactor[1];
					capsum[0] = capsum[1] = temp;
				}
				else
				{
					capsum[0] = +root->cap * capScaleFactor[1] * root->localrc[1];
					capsum[1] = +root->cap * capScaleFactor[1] * root->localrc[2];
				}

			}
			else
			{
				if (!betaCalc)
				{
					double pindap[2];
					findPinCap ( root, pindap );

					temp = capsum[0] + root->cap * capScaleFactor[1];
					capsum[0] = temp - (pindap[0] * capScaleFactor[1]) + pindap[0];
					capsum[1] = temp - (pindap[1] * capScaleFactor[1]) + pindap[1];
				}
				else
				{
					double pinCap[] =
					{ 0.0, 0.0 };
					findPinCap ( root, pinCap );

					capsum[0] += (root->cap - pinCap[0]) * capScaleFactor[1] + pinCap[0];
					capsum[1] += (root->cap - pinCap[1]) * capScaleFactor[1] + pinCap[1];
				}
			}
		}
		else		//Nominal Case
		{
			if (!betaCalc)
			{
				temp = capsum[0] + root->cap;
				capsum[0] = capsum[1] = temp;
			}
			else
			{
				temp = capsum[0] + root->cap * root->localrc[0];
				capsum[0] = capsum[1] = temp;
			}
		}

		interm[0] = capsum[0];
		interm[1] = capsum[1];

		for (i = 0; i < root->len; i++)
			sumCaps ( root->linkedFw[i], 0, corner, betaCalc, capsum );
	}

}

/******************
 * Main Algorithm *
 ******************/

void elmorePath ( path *pathTap, node *pin, char *tapId, int corner, double delay[] )
{
	double temp;
	pathnode *cur;
	int i;

//Find path to tap
	pathFind ( pathTap, pin, pin->res, tapId );

//Push input pin
	pushPath ( pathTap, pin, 0, 0.0 );

#if INTERC_DBG
	printPath ( pathTap );
#endif

	//printPath ( pathTap );
	//fflush ( stdout );

	if (!strcmp ( pathTap->top->mark->id, pathTap->base->mark->id ))
		return;

//Compute Delay

	if (!corner)
	{
		for (cur = pathTap->top; cur != NULL ; cur = cur->prev)
		{
			//Compute the path Missing
			temp = delay[0] + cur->mark->cap * cur->resSum;
			delay[0] = delay[1] = temp;

			if (cur->mark->localrc[0] == 0)
				cur->mark->localrc[0] = temp;

			//Compute other subtrees
			if (cur->mark->len > 1)
			{
				int id = cur->prev->indexfound;
				double capSum[] =
				{ 0.0, 0.0 };

				for (i = 0; i < cur->mark->len; i++)
				{
					if (i != id)
					{
						sumCaps ( cur->mark->linkedFw[i], 1, NOMINAL, 0, capSum );

#if INTERC_DBG
						printf ( "$$#cap sum: %e\n", capSum[0] );
#endif
						//Compute delays
						temp = delay[0] + capSum[0] * cur->resSum;
						delay[0] = delay[1] = temp;
					}
				}
			}
		}
	}

//Delay in sigma corner
	if (corner)
	{
		for (cur = pathTap->top; cur != NULL ; cur = cur->prev)
		{
			//Compute the path
			if (cur->mark->len > 0)
			{
				temp = delay[0] + cur->mark->cap * capScaleFactor[1] * cur->resSum * resScaleFactor[1];
				delay[0] = delay[1] = temp;
			}
			else
			{
				double pinCap[] =
				{ 0.0, 0.0 };
				findPinCap ( cur->mark, pinCap );

				delay[0] += ((cur->mark->cap - pinCap[0]) * capScaleFactor[1] + pinCap[0]) * cur->resSum * resScaleFactor[1];
				delay[1] += ((cur->mark->cap - pinCap[1]) * capScaleFactor[1] + pinCap[1]) * cur->resSum * resScaleFactor[1];
			}

			//cur->mark->localrc[1] = (cur->mark->localrc[1] == 0) ? delay[0] : (cur->mark->localrc[1] += 0.0);
			//cur->mark->localrc[2] = (cur->mark->localrc[2] == 0) ? delay[1] : (cur->mark->localrc[2] += 0.0);

			if (cur->mark->localrc[1] == 0)
				cur->mark->localrc[1] = delay[0];

			if (cur->mark->localrc[2] == 0)
			{
				cur->mark->localrc[2] = delay[1];

				//Integrate at @each node
				gaussianDist temp_rise, temp_fall;

				gauss_set ( &temp_fall, cur->mark->localrc[0], (cur->mark->localrc[1] - cur->mark->localrc[0]) / 3 );
				gauss_set ( &temp_rise, cur->mark->localrc[0], (cur->mark->localrc[2] - cur->mark->localrc[0]) / 3 );

				add2Gaussians ( &pathTap->top->mark->at->erlFall, &temp_fall, &cur->mark->at->erlFall );
				add2Gaussians ( &pathTap->top->mark->at->erlRise, &temp_rise, &cur->mark->at->erlRise );
				add2Gaussians ( &pathTap->top->mark->at->ltFall, &temp_fall, &cur->mark->at->ltFall );
				add2Gaussians ( &pathTap->top->mark->at->erlRise, &temp_rise, &cur->mark->at->erlRise );

			}

			//Compute other subtrees
			if (cur->mark->len > 1)
			{
				int id = cur->prev->indexfound;
				double capSum[] =
				{ 0.0, 0.0 };

				for (i = 0; i < cur->mark->len; i++)
				{
					if (i != id)
					{
						sumCaps ( cur->mark->linkedFw[i], 1, SIGMA, 0, capSum );
#if INTERC_DBG
						printf ( "$$#cap sum: %e\n", capSum[0] );
#endif
						//delay += capSum * cur->resSum * resScaleFactor[1];
						delay[0] += capSum[0] * cur->resSum * resScaleFactor[1];
						delay[1] += capSum[1] * cur->resSum * resScaleFactor[1];
					}
				}
			}
		}
	}

}

void treeDelay ( interInfo *info )
{
	int i;
	path *track;

	track = initpath ();

	for (i = 0; i < info->nofTaps; ++i)
	{
		double temp_nom[2];
		double temp_sig[2];
		double alpha[2];

		elmorePath ( track, info->netStart, info->netEnd[i]->id, NOMINAL, temp_nom );

		emptyPath ( track );

		elmorePath ( track, info->netStart, info->netEnd[i]->id, SIGMA, temp_sig );

		alpha[0] = (temp_sig[0] - temp_nom[0]) / 3;
		alpha[1] = (temp_sig[1] - temp_nom[1]) / 3;

		//they need change to distribution data types with nominal value: temp_nom and variance
		gauss_set ( &info->delay[i][0], temp_nom[0], alpha[0] );
		gauss_set ( &info->delay[i][1], temp_nom[1], alpha[1] );

		//info->delay[i][0] = temp_nom[0] + alpha[0] * 0.0000000001435;
		//info->delay[i][1] = temp_nom[1] + alpha[1] * 0.0000000001435;

		emptyPath ( track );

	}

	free ( track );

}

/*********************************
 * 	Slack Computation Functions  *
 *********************************/

double *betaCalc ( path *pathTap, node *pin, char *tapId )
{

	double *beta;
	double temp_sig;
	pathnode *cur;
	int i;

	//allocate vectors
	beta = (double *) calloc ( 3, sizeof(double) );

	//Find path to tap
	pathFind ( pathTap, pin, pin->res, tapId );

	//Push input pin
	pushPath ( pathTap, pin, 0, 0.0 );

	if (!strcmp ( pathTap->top->mark->id, pathTap->base->mark->id ))
		return beta;

	for (cur = pathTap->top; cur != NULL ; cur = cur->prev)
	{
		//Compute the path
		if (cur->mark->len > 0)
		{
			temp_sig = beta[1] + cur->mark->cap * capScaleFactor[1] * cur->resSum * resScaleFactor[1];
			beta[1] = beta[2] = temp_sig;

			//Nominal beta
			beta[0] += cur->mark->cap * cur->resSum;

		}
		else
		{
			//Nominal beta
			beta[0] += cur->mark->cap * cur->resSum;

			double pinCap[] =
			{ 0.0, 0.0 };

			findPinCap ( cur->mark, pinCap );

			beta[1] += ((cur->mark->cap - pinCap[0]) * capScaleFactor[1] + pinCap[0]) * cur->resSum * resScaleFactor[1];
			beta[2] += ((cur->mark->cap - pinCap[1]) * capScaleFactor[1] + pinCap[1]) * cur->resSum * resScaleFactor[1];
		}

		//Compute other subtrees
		if (cur->mark->len > 1)
		{
			int id = cur->prev->indexfound;
			double capSum[2] =
			{ 0.0, 0.0 };

			for (i = 0; i < cur->mark->len; i++)
			{
				if (i != id)
				{
					//Nominal beta
					sumCaps ( cur->mark->linkedFw[i], 1, NOMINAL, 1, capSum );
					beta[0] += capSum[0] * cur->resSum;

					sumCaps ( cur->mark->linkedFw[i], 1, SIGMA, 1, capSum );
#if INTERC_DBG
					printf ( "$$#cap sum for beta: %e\n", capSum[0] );
#endif
					//delay += capSum * cur->resSum * resScaleFactor[1];
					beta[1] += capSum[0] * cur->resSum * resScaleFactor[1];
					beta[2] += capSum[1] * cur->resSum * resScaleFactor[1];

				}
			}
		}
	}

	return beta;
}

void commitInfo ( interInfo *nfo )
{
	unsigned int i;

	for (i = 0; i < nfo->nofTaps; ++i)
	{

		add2Gaussians ( &nfo->netStart->at->erlFall, &nfo->delay[i][0], &nfo->netEnd[i]->at->erlFall );
		add2Gaussians ( &nfo->netStart->at->erlRise, &nfo->delay[i][1], &nfo->netEnd[i]->at->erlRise );
		add2Gaussians ( &nfo->netStart->at->ltFall, &nfo->delay[i][0], &nfo->netEnd[i]->at->ltFall );
		add2Gaussians ( &nfo->netStart->at->ltRise, &nfo->delay[i][1], &nfo->netEnd[i]->at->ltRise );

		if (isnan ( nfo->netStart->slew->erlFall.mean ) || isnan (nfo->slew[i][0].mean))
			continue;
		if (isnan ( nfo->netStart->slew->erlRise.mean ) || isnan(nfo->slew[i][1].mean))
			continue;
		if (isnan ( nfo->netStart->slew->ltFall.mean ) || isnan(nfo->slew[i][2].mean))
			continue;
		if (isnan ( nfo->netStart->slew->ltRise.mean ) || isnan(nfo->slew[i][3].mean))
			continue;

		add2Gaussians ( &nfo->netStart->slew->erlFall, &nfo->slew[i][0], &nfo->netEnd[i]->slew->erlFall );
		add2Gaussians ( &nfo->netStart->slew->erlRise, &nfo->slew[i][1], &nfo->netEnd[i]->slew->erlRise );
		add2Gaussians ( &nfo->netStart->slew->ltFall, &nfo->slew[i][2], &nfo->netEnd[i]->slew->ltFall );
		add2Gaussians ( &nfo->netStart->slew->ltRise, &nfo->slew[i][3], &nfo->netEnd[i]->slew->ltRise );

	}

}

void treeSlew ( interInfo *info )
{
	int i;
	double **beta;
	double *impulseSlew_nom;
	//double *impulseSlew_sig;
	double **rampSlew_nom;
	double **rampSlew_sig;
	double **alpha_pulse;
	double **alpha_ramp;

	node *port = info->netStart;

	impulseSlew_nom = (double *) malloc ( info->nofTaps * sizeof(double) );  //TODO needs free
	//impulseSlew_sig = (double *) malloc ( info->nofTaps * sizeof(double) );  //TODO needs free
	rampSlew_nom = (double **) malloc ( info->nofTaps * sizeof(double *) );  //TODO needs free and fix dimensions
	rampSlew_sig = (double **) malloc ( info->nofTaps * sizeof(double *) );
	beta = (double **) malloc ( info->nofTaps * sizeof(double *) );
	alpha_pulse = (double **) malloc ( info->nofTaps * sizeof(double *) );
	alpha_ramp = (double **) malloc ( info->nofTaps * sizeof(double *) );

	//compute betas

	path *track;

	track = initpath ();

	for (i = 0; i < info->nofTaps; ++i)
	{
		beta[i] = betaCalc ( track, info->netStart, info->netEnd[i]->id );
		emptyPath ( track );
	}

	free ( track );

	//Compute Slews

	for (i = 0; i < info->nofTaps; ++i)
	{
		impulseSlew_nom[i] = sqrt ( 2 * beta[i][0] - (info->netEnd[i]->localrc[0] * info->netEnd[i]->localrc[0]) );

		rampSlew_nom[i] = (double *) malloc ( 4 * sizeof(double) );
		rampSlew_sig[i] = (double *) malloc ( 2 * sizeof(double) );  //only rise/fall
		alpha_pulse[i] = (double *) malloc ( 4 * sizeof(double) );
		alpha_ramp[i] = (double *) malloc ( 4 * sizeof(double) );

		rampSlew_nom[i][0] = sqrt (
				port->slew->erlFall.mean * port->slew->erlFall.mean + impulseSlew_nom[i] * impulseSlew_nom[i] );
		rampSlew_nom[i][1] = sqrt (
				port->slew->erlRise.mean * port->slew->erlRise.mean + impulseSlew_nom[i] * impulseSlew_nom[i] );
		rampSlew_nom[i][2] = sqrt (
				port->slew->ltFall.mean * port->slew->ltFall.mean + impulseSlew_nom[i] * impulseSlew_nom[i] );
		rampSlew_nom[i][3] = sqrt (
				port->slew->ltRise.mean * port->slew->ltRise.mean + impulseSlew_nom[i] * impulseSlew_nom[i] );

		rampSlew_sig[i][0] = sqrt ( (2 * beta[i][1]) - (info->netEnd[i]->localrc[1] * info->netEnd[i]->localrc[1]) );  //fall
		rampSlew_sig[i][1] = sqrt ( (2 * beta[i][2]) - (info->netEnd[i]->localrc[2] * info->netEnd[i]->localrc[2]) );  //rise

		alpha_pulse[i][0] = (rampSlew_sig[i][0] - rampSlew_nom[i][0]) / 3;  //early fall
		alpha_pulse[i][1] = (rampSlew_sig[i][1] - rampSlew_nom[i][1]) / 3;  //early rise
		alpha_pulse[i][2] = (rampSlew_sig[i][0] - rampSlew_nom[i][2]) / 3;  //late fall
		alpha_pulse[i][3] = (rampSlew_sig[i][1] - rampSlew_nom[i][3]) / 3;  //late rise

		alpha_ramp[i][0] = (sqrt (
				pow ( (port->slew->erlFall.mean + 3 * port->slew->erlFall.sigma), 2 )
						+ pow ( (rampSlew_nom[i][0] + 3 * alpha_pulse[i][0]), 2 ) ) - rampSlew_sig[i][0]) / 3;  //early fall
		alpha_ramp[i][1] = (sqrt (
				pow ( port->slew->erlRise.mean + 3 * port->slew->erlRise.sigma, 2 ) + pow ( rampSlew_nom[i][1] + 3 * alpha_pulse[i][1], 2 ) )
				- rampSlew_sig[i][1]) / 3;  //early rise
		alpha_ramp[i][2] = (sqrt (
				pow ( port->slew->ltFall.mean + 3 * port->slew->ltFall.sigma, 2 )
						+ pow ( rampSlew_nom[i][2] + 3 * alpha_pulse[i][2], 2 ) ) - rampSlew_sig[i][0]) / 3;	 //late fall
		alpha_ramp[i][3] = (sqrt (
				pow ( port->slew->ltRise.mean + 3 * port->slew->ltRise.sigma, 2 )
						+ pow ( rampSlew_nom[i][3] + 3 * alpha_pulse[i][3], 2 ) ) - rampSlew_sig[i][1]) / 3;	 //late rise

		if (isnan ( alpha_ramp[i][0] ))
			alpha_ramp[i][0] = rampSlew_nom[i][0] / 30;

		if (isnan(alpha_ramp[i][1]))
			alpha_ramp[i][1] = rampSlew_nom[i][1] / 30;

		if (isnan(alpha_ramp[i][2]))
			alpha_ramp[i][2] = rampSlew_nom[i][2] / 30;

		if (isnan(alpha_ramp[i][3]))
			alpha_ramp[i][3] = rampSlew_nom[i][3] / 30;

		//store to info //NUMERICAL ISSUE
		gauss_set ( &info->slew[i][0], rampSlew_nom[i][0], alpha_ramp[i][0] );
		gauss_set ( &info->slew[i][1], rampSlew_nom[i][1], alpha_ramp[i][1] );
		gauss_set ( &info->slew[i][2], rampSlew_nom[i][2], alpha_ramp[i][2] );
		gauss_set ( &info->slew[i][3], rampSlew_nom[i][3], alpha_ramp[i][3] );

//		info->slew[i][0] = rampSlew_nom[i][0] + (alpha_ramp[i][0] * 0.0000001);
//		info->slew[i][1] = rampSlew_nom[i][1] + (alpha_ramp[i][1] * 0.0000000001);
//		info->slew[i][2] = rampSlew_nom[i][2] + (alpha_ramp[i][2] * 0.0000001);
//		info->slew[i][3] = rampSlew_nom[i][3] + (alpha_ramp[i][3] * 0.00000000001);
	}

	for (i = 0; i < info->nofTaps; ++i)
	{
		free ( rampSlew_nom[i] );
		free ( alpha_pulse[i] );
		free ( alpha_ramp[i] );
		free ( rampSlew_sig[i] );
	}

	free ( impulseSlew_nom );
	free ( rampSlew_nom );
	free ( alpha_pulse );
	free ( alpha_ramp );
	free ( rampSlew_sig );

}

void printInfo ( interInfo *nfo, int all )
{
	unsigned int i;

	printf ( "\n@@ Net Info:\n" );
	printf ( "\t-# path input:%s\n", nfo->netStart->id );
	printf ( "\n\t-# number of taps:%d\n", nfo->nofTaps );

	for (i = 0; i < nfo->nofTaps; ++i)
		printf ( "\t-# path tap:%s\n", nfo->netEnd[i]->id );

	if (all)
	{
		for (i = 0; i < nfo->nofTaps; ++i)
		{
			printf ( "\n\t-------------------------\n" );

			printf ( "\t-# rise delay:\t%.15e, %.15e\n", nfo->delay[i][0].mean, nfo->delay[i][0].sigma );
			printf ( "\t-# fall delay:\t%.15e, %.15e\n", nfo->delay[i][1].mean, nfo->delay[i][1].sigma );

			printf ( "\n\t-# early fall slew:\t%.15e, %.15e\n", nfo->slew[i][0].mean, nfo->slew[i][0].sigma );
			printf ( "\t-# early rise slew:\t%.15e, %.15e\n", nfo->slew[i][1].mean, nfo->slew[i][1].sigma );
			printf ( "\t-# late fall slew:\t%.15e, %.15e\n", nfo->slew[i][2].mean, nfo->slew[i][2].sigma );
			printf ( "\t-# late rise slew:\t%.15e, %.15e\n", nfo->slew[i][3].mean, nfo->slew[i][3].sigma );
		}

	}

}

/******************************************************
 * 	Main Interconnection Characteristics Calculation  *
 ******************************************************/

interInfo *interconnectCalc ( node *pin )
{
	interInfo *inter = NULL;

	if (pin->netEndPntCnt != 0)
		inter = initInterInfo ( pin, pin->netEndPntCnt );

#if INTERC_DBG
	else
	printf ( "@@Could not Initialize interconnect Information.Wrong Pin\n" );
#endif

	if (inter)
	{
		//printInfo ( inter, 0 );

		treeDelay ( inter );

		treeSlew ( inter );

		commitInfo ( inter );

		//printInfo ( inter, 1 );

	}

	return inter;

}

