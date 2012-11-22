#include "edfomai-data.h"
#include <native/task.h>

/*
* 
*/
int init_edfomai();
/*
* 
*/
int dispose_edfomai();
/*
* 
*/
int edf_task_start( RT_TASK * task, RTIME deadline, void (*procedure)(void *arg), void * arg );
/*
* 
*/
int edf_task_wait_period( unsigned long * overruns_r );


