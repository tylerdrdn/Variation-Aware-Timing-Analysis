#ifndef TYPES_H_
#define TYPES_H_

/****************************************************************
*						flip flop fields						*
*****************************************************************/


typedef struct _ffClk
{
    char *pin;								// 32 chars
    double fallCap;
    double riseCap;
}
ffClk;

typedef struct _fallConstraint				//function No 29
{
    double g;
    double h;
    double j;
}
fallConstraint;

typedef struct _riseConstraint				//function No 30
{
    double m;
    double n;
    double p;
}
riseConstraint;

typedef enum _edgeType
{
	FALLING = 0,
	RISING = 1
}
edgeType;

typedef struct _setup
{
    char *clockPin; 						// 32 chars
    char *inputPin; 						// 32 chars
    edgeType edge; 							// rising/falling

    fallConstraint *fallConstr;
    riseConstraint *riseConstr;
}
setup;

typedef struct _hold
{
    char *clkPin; 							// 32 chars
    char *inpPin; 							// 32 chars
    edgeType edge; 							// rising/falling

    fallConstraint *fallConstr;
    riseConstraint *riseConstr;
}
hold;

typedef struct _ffStruct
{
    ffClk *clock;
    setup **setup;
    hold **hold;
    int setupCnt;
    int holdCnt;
}
ffStruct;

/****************************************************************
*						 cell fields							*
*****************************************************************/

typedef struct _slewDef
{
  double x;
  double y;
  double z;
  double ksv;
  double kst;
  double ksl;
  double ksw;
  double ksh;
  double ksr;
}
slewDef;

typedef struct _delay
{
  double a;
  double b;
  double c;
  double kdv;
  double kdt;
  double kdl;
  double kdw;
  double kdh;
  double kdr;
}
delay;

typedef struct _inPins
{
    char *pin;								// 32 chars
    double fallCap;
    double riseCap;
}
inPins;

typedef struct _outPin
{
    char *pin;								// 32 chars
}
outPin;

typedef enum _sense
{
	NON_U = 0,
	POSITIVE_U = 1,
	NEGATIVE_U = 2
}
sense;

typedef struct _timing
{
    char *inpPin; 							// 32 chars
    char *outPin; 							// 32 chars
    sense timingSense;

    slewDef *fallSlew;
    slewDef *riseSlew;

    delay *fallDelay;
    delay *riseDelay;
}
timing;

typedef struct _cell
{
    char *type;

    inPins **inputPins;
    int inPinsCnt;

    outPin **outPins;
    int	 outPinsCnt;

    timing **timing;
    int timingsCnt;

    ffStruct *flipFlop;

    struct cell *next;
}
cell;

typedef struct _libNode
{
	char **keys;
	struct _libNode **objList;
	cell **leafCells;

	int size;
	int level;
}
libNode;

/*****************************************************************
*					Netlist Related Fields 				 		 *
******************************************************************/

typedef enum _transition
{
	FALL = 0,
	RISE = 1
}
transition;

typedef struct _slew
{
    double erlFall;
    double erlRise;
    double ltFall;
    double ltRise;
}
slew;

typedef struct _at
{
    double erlFall;
    double ltFall;
    double erlRise;
    double ltRise;
}
at; // arrival time

typedef struct _csrt
{
	struct _node *node;
	char *opMode;
	unsigned int numOfCsrt;
	double ratFall;
	double ratRise;
}
csrt;

typedef struct _res
{
	char *nodesID[2];
	double value;
	struct _res *nxt;
}
res;

typedef struct _cap
{
	char *nodeID;
	double value;
	struct _cap *nxt;
}
cap;

typedef struct clock
{
	struct _node *clkNode;
	double period;
}
clk;

typedef struct _fflop
{
	double t_setup[2];						/* for each transition {fall, rise} */
	double t_hold[2];						/* for each transition {fall, rise} */
	at *slack;
}
fflop;

typedef struct _node
{
	char *id;								/* node's name */
	char *type;								/* node's type (cell pin, wire pin etc.) */
	cell *cellType;							/* type of cell */

	double cap;								/* node's capacitance */
	double res;								/* the res between the node & the previous node */

	struct _node **linkedFw;				/* node is connected to nodes that follow */
	struct _node *linkedBw;					/* node is connected to previous nodes */
	struct _node *netStartNode;				/* if node servers as a terminal node of a net then this pointer points to the beginning of the net */
	struct _node **netEndNodes;				/* if node serves as the begining of a net then these pointers point to the end nodes of the net */

	int len;								/* how many nodes follow? */
	state visited;							/* stores the state of the node whether it has been visited through the BFS step */

	int netEndPntCnt;

	slew *slew;								/* node's transition time */
	at *at;									/* arrival time at node */
	delay *df;								/* fall delay for the vertex that ends to this node */
	delay *dr;								/* rise delay for the vertex that ends to this node */

	struct _node *x_nxt;					/* next pointer to a node in a list of nodes */
	struct _node *next;						/* next pointer to a node in each list of Hash Table */
}
node;

typedef struct _queue
{
	struct _qnode *head;
	struct _qnode *tail;
}
queue;

typedef struct _qnode
{
	struct _node *node;
	struct _qnode *nxt;
}
qnode;

/*****************************************************************
*					Timing Graph Related Fields 				 *
******************************************************************/

typedef enum _state
{
	FALSE = 0,
	TRUE = 1
}
state;

#endif /* TYPES_H_ */
