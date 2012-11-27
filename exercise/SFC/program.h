
#define PLCperiod 8*1e9
#define nStep 3 //number of Steps

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
