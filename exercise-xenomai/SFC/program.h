
#define PLCperiod 8*1e9	// PLC period length
#define nStep 3		// number of Steps

short stepStatus[nStep];
void* (*step[nStep];(void*);
void* (*condition[nStep])(void*);

/*
 * void* readInputs():
 * return the pointer to read inputs
 */
void* readInputs();

/*
 * void writeOutputs(void*):
 * put the given data to outputs
 */
void writeOutputs(void*);


/*
 * void stepInitializzator():
 * initialize stepStatus, step and condition arrays
 */
void stepInitializzator();

