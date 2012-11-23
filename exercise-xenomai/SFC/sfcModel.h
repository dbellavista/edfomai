
typedef enum {
	N,
	P,
	S,
	R
} qualifier;

typedef struct {
	qualifier type;
	// actionDefinition;
} action;

typedef enum {
	condition,
	exclusiveChoice,
	convergence,
	parallel,
	synchronization
} structureType;

typedef struct {
	structureType type;
	void* structure;
} structure;

typedef struct {
	short state;	// 0 = inactive, 1 = active
	int nActions;	// number of actions
	action* actions;
	structure link;
} step;

typedef struct {
	step predecessor;
	step successor;
	short (*check)(void*,void*);	// it gets sensors and actuators and return 0 if the check doesn't pass
} sCondition;

typedef struct {
	step predecessor;
	step *successors;
	short (**check)(void*,void*);
} sExclusiveChoice;

typedef struct {
} sConvergence;

typedef struct {
} sParallel;

typedef struct {
} sSynchronization;


