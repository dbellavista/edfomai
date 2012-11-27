#include "edfomai-data.h"
#include <native/task.h>

/*
* Initialize EDFOMAI resources - (NOT THREAD SAFE)
*/
int init_edfomai();
/*
* Shutdown and destroy all resources associated to EDFOMAI
*/
int dispose_edfomai();
/*
* Start an RT_TASK with deadline
* Deadline is relative and might be interpreted as the maximum task duration permitted.
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
* Setting deadline to DEADLINENOTSET will start the task with no deadline, so 
* priority remains static (could be usefull in compination with other unimplemented APIs)
*/
int edf_task_start( RT_TASK * task, unsigned long deadline, void (*procedure)(void *arg), void * arg );
/*
* Wait a period (for periodic tasks only!)
* Then deadline will be resetted to starting specified value.
*/
int edf_task_wait_period( unsigned long * overruns_r );


