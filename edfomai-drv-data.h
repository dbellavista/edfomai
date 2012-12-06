#ifndef _EDFOMAI_DRV_DATA_
#define _EDFOMAI_DRV_DATA_

#ifdef MODULE
#include <rtdm/rtdm_driver.h>
#include <native/task.h>
#else
#include <stdlib.h>
#include <native/task.h>
#include <native/alarm.h>
#endif

#include "edfomai-data.h"

/* 
* NOTE: the mutex with this name SHOULD be created and registered 
* by the kernel module.
*/
#define DMUTEX_NAME "EDF_DATA_MUTEX"

#define init_dtask( dtask ) { \
			dtask->deadline=DEADLINENOTSET; \
			dtask->remain=DEADLINENOTSET; \
			dtask->relative_deadline=DEADLINENOTSET; \
			dtask->state=OK; \
			dtask->task=NULL; \
			dtask->watchd=(RT_ALARM *)rtdm_malloc(sizeof(RT_ALARM)); \
	}
#define reset_watchd( dtask ) { \
			dtask->watchd; \
			;\
	}

#define clear_dtask( dtask ) { \
			rtdm_free(dtask->watchd); \
	}

/*
* 
*/
typedef enum DeadlineState{
	OK=0,
	MISSED=1
} DeadlineState;
/*
* Structure that wrap an RT_TASK and permit deadline handling
*/
typedef struct rt_deadline_task {
	unsigned long deadline;
	unsigned long remain;
	unsigned long relative_deadline;
	DeadlineState state;
	RT_TASK * task;
	RT_ALARM * watchd;
	RT_TASK_INFO task_info;
} RT_DEADLINE_TASK;

/*
* Recalculate tasks priorities considering deadline distance
*/
int rt_dtask_recalculateprio(void);
/*
* Update the TASL_INFO structue of the associated RT_TASK
*/
int rt_dtask_updateinfo ( RT_TASK * task );
/*
* Update the TASL_INFO structue of the associated RT_TASK
*/
int rt_dtask_stopwatchdog ( RT_TASK * task );
/*
* Reset task's deadline with previous setted value.
*/
int rt_dtask_resetdeadline( RT_TASK * task);
/*
* Set task's deadline. New deadline is meant to be a relative 
* deadline value (eg. task deadline is an 200000ns since it's start or modification)
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
*/
int rt_dtask_setdeadline(RT_TASK * task, unsigned long newdeadline);
/*
* Create a Task with deadline specification.
* Deadline is relative and might be interpreted as the maximum task duration permitted.
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
*/
int rt_dtask_create ( RT_TASK * task, unsigned long deadline );
/*
* Delete a RT_TASK.
* This exclude the specified RT_TASK from the EDF based priority rearrangment.
* Latest setted priority are maintained so this procedure should be invoked specifying
* a DELETED task only.
*/
int rt_dtask_delete ( RT_TASK * task );
/*
* Initialize the data structures needed for handling EDF based priorities arrangement.
*/
int rt_dtask_init(void);
/*
* Free all data structures.
*/
int rt_dtask_dispose(void);

#endif
