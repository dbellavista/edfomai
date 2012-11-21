#include <linux/string.h> 
#include "edfomai-drv-data.h"
#include "datastructures/uthash.h"

#define MAXPRIO RTDM_TASK_HIGHEST_PRIORITY
#define MINPRIO RTDM_TASK_LOWER_PRIORITY

/* 
* RT_DEADLINE_TASK managable by uthash
*/
typedef struct rt_dtask{
	RT_DEADLINE_TASK dtask;
	UT_hash_handle hh;
} RT_DTASK;

/* Used by uthash for referencing the hash table */
static RT_DTASK * dtask_map = NULL;

/*
* Returns the priority associated with the remaining time 
*/
static inline int _calculate_prio( nanosecs_abs_t remain, nanosecs_abs_t min_remain, 
			nanosecs_abs_t max_remain){
	return (int) ( remain * (MAXPRIO-MINPRIO) / (max_remain-min_remain) );
}
/*
* Recalculate tasks priorities considering deadline distance
* Complexity O(N+N). 
* Further version should improve performance reaching a 
* better complexity.
*/
int rt_dtask_recalculateprio(){
	RT_DTASK * rtdtask, * tmp;
	unsigned int newprio;
	nanosecs_abs_t remain , min_remain=0, max_remain=0;
	HASH_ITER(hh, dtask_map, rtdtask, tmp) {
		if (rtdtask->dtask.relative_deadline != DEADLINENOTSET){
			rtdtask->dtask.remain = rtdtask->dtask.deadline - rtdm_clock_read();
			min_remain = ( remain>=0 && remain < min_remain ? remain : min_remain );
			max_remain = ( remain>=0 && remain > max_remain ? remain : max_remain );
		}
	}
	HASH_ITER(hh, dtask_map, rtdtask, tmp) {
	        newprio=_calculate_prio(rtdtask->dtask.remain, min_remain, max_remain );
		rt_task_set_priority( &(rtdtask->dtask.task) , newprio );
	}
	return 0;
}
/*
* Update the TASL_INFO structue of the associated RT_TASK
*/
int rt_dtask_updateinfo ( RT_TASK * task ){
	int ret;
	RT_DTASK * rtdtask;
	HASH_FIND_STR(dtask_map, task->rname, rtdtask);
	ret=rt_task_inquire( &(rtdtask->dtask.task) , &(rtdtask->dtask.task_info) );
	return ret;
}
/*
* Reset task's deadline with previous setted value.
*/
int rt_dtask_resetdeadline( RT_TASK * task){
	RT_DTASK * rtdtask;
	HASH_FIND_STR(dtask_map, task->rname, rtdtask);
	rtdtask->dtask.deadline = rtdm_clock_read() + rtdtask->dtask.relative_deadline;
	return 0;
}
/*
* Set task's deadline. New deadline is meant to be a relative 
* deadline value (eg. task deadline is an 200000ns since it's start or modification)
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
*/
int rt_dtask_setdeadline(RT_TASK * task, nanosecs_abs_t newdeadline){
	RT_DTASK * rtdtask;
	HASH_FIND_STR(dtask_map, task->rname, rtdtask);
	rtdtask->dtask.relative_deadline = newdeadline;
	rtdtask->dtask.deadline = rtdm_clock_read() + rtdtask->dtask.relative_deadline;
	return 0;
}
/*
* Create a Task with deadline specification.
* Deadline is relative and might be interpreted as the maximum task duration permitted.
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
*/
int rt_dtask_create ( RT_TASK * task, nanosecs_abs_t deadline ){

	RT_DTASK * rtdtask = (RT_DTASK*) rtdm_malloc(sizeof(RT_DTASK));
	memcpy( &(rtdtask->dtask.task) , task , sizeof(RT_TASK));
	rtdtask->dtask.relative_deadline = deadline;
	rtdtask->dtask.deadline = rtdm_clock_read() + rtdtask->dtask.relative_deadline;
	HASH_ADD_STR( dtask_map, dtask.task.rname , rtdtask);
	return 0;
}
/*
* Delete a RT_TASK.
* This exclude the specified RT_TASK from the EDF based priority rearrangment.
* Latest setted priority are maintained so this procedure should be invoked specifying
* a DELETED task only.
*/
int rt_dtask_delete ( RT_TASK * task ){
	RT_DTASK * rtdtask;
	HASH_FIND_STR(dtask_map, task->rname, rtdtask);
	HASH_DEL(dtask_map, rtdtask );
	rtdm_free(rtdtask);
	return 0;
}
/*
* Initialize the data structures needed for handling EDF based priorities arrangement.
*/
int rt_dtask_init(){
	dtask_map = NULL;
	return 0;
}
/*
* Free all data structures.
*/
int rt_dtask_dispose(){
	RT_DTASK * rtdtask;
	RT_DTASK * tmp;
	HASH_ITER(hh, dtask_map, rtdtask, tmp) {
	        HASH_DEL(dtask_map,rtdtask);
	        rtdm_free(rtdtask);
	}	
	return 0;
}


