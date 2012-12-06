#include "edfomai-drv-data.h"
#ifdef MODULE
#include <linux/string.h> 
#else
#include <string.h>
#endif
#include <native/timer.h>
#include <native/task.h>
#include <native/alarm.h> 

#include "datastructures/uthash.h"

#define MAXPRIO 50 //RTDM_TASK_HIGHEST_PRIORITY
#define MINPRIO 0  //RTDM_TASK_LOWER_PRIORITY

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
* Anyone who wants to use this edfomai data structures should define a procedure
* having the signature above
*/
extern void deadline_missed( struct rt_alarm *alarm, void * cookie );

/*
* Returns the priority associated with the remaining time 
*/
static inline int _calculate_prio( unsigned long remain, 
			unsigned long min_remain, unsigned long max_remain){
	return (unsigned int) MAXPRIO - (unsigned int) ( (remain-min_remain) * (MAXPRIO-MINPRIO) / 
		( (max_remain-min_remain)==0? (remain?remain:1) : max_remain-min_remain ));
}
/*
* Recalculate tasks priorities considering deadline distance
* Complexity O(N+N). 
* Further version should improve performance reaching a 
* better complexity.
*/
int rt_dtask_recalculateprio(){
	RT_DTASK * rtdtask, * tmp;
	RT_TASK_INFO task_info;
	unsigned int newprio;
	unsigned long curr_time, min_remain=0xffffffffffffffff, max_remain=0; // 2^64 -1
	int count=0, missed=0;
	count=HASH_COUNT(dtask_map);
	#ifdef DEBUG
	rtdm_printk("Edfomai: [@dt_rcalcp] recalculating priorities of %d tasks\n",count);
	#endif
	if (!count)
		return 0;

	curr_time=(unsigned long)rt_timer_read();
	HASH_ITER(hh, dtask_map, rtdtask, tmp) {
		if (rtdtask->dtask.relative_deadline != DEADLINENOTSET && rtdtask->dtask.status==OK ){
			rtdtask->dtask.remain=(rtdtask->dtask.deadline > curr_time ?
							rtdtask->dtask.deadline - curr_time : 0);
			if (rtdtask->dtask.remain==0)
				missed++;
			min_remain=(rtdtask->dtask.remain < min_remain ? rtdtask->dtask.remain : min_remain );
			max_remain=(rtdtask->dtask.remain > max_remain ? rtdtask->dtask.remain : max_remain );
			rt_task_inquire(rtdtask->dtask.task,&task_info);
			rt_task_inquire(rtdtask->dtask.task, &(rtdtask->dtask.task_info));
		}else if (rtdtask->dtask.status==MISSED){
			missed++;
		}
	}
	#ifdef DEBUG
	rtdm_printk("Edfomai: [@dt_rcalcp] #tasks(%d) #missed(%d)\n",count,missed);
	#endif
	HASH_ITER(hh, dtask_map, rtdtask, tmp) {
		if ( rtdtask->dtask.relative_deadline!=DEADLINENOTSET && rtdtask->dtask.status==OK ){
			if (rtdtask->dtask.remain==0){
				rtdtask->dtask.status=MISSED;
				#ifdef DEBUG
				rtdm_printk("Edfomai: [@dt_rcalcp] task (%s) has missed his deadline\n",rtdtask->dtask.task->rname);
				#endif
				deadline_missed(NULL,rtdtask->dtask.task);
			}
			newprio=_calculate_prio(rtdtask->dtask.remain, min_remain, max_remain );
			#ifdef DEBUG
			rtdm_printk("Edfomai: [@dt_rcalcp] task(%s) oldP(%u) newP(%u) remain(%lu) maxRem(%lu) minRem=(%lu)\n",
								rtdtask->dtask.task_info.name, rtdtask->dtask.task_info.cprio, 
								newprio, rtdtask->dtask.remain ,max_remain, min_remain );
			#endif
			if (newprio!=rtdtask->dtask.task_info.cprio)
				rt_task_set_priority( rtdtask->dtask.task , newprio );
		}
	}
	return 0;
}
/*
* Reset the watchdog of the specified task
*/
int rt_dtask_stopwatchdog ( RT_TASK * task ){
	int ret=-1;
	RT_DTASK * rtdtask;
	RT_TASK_INFO * task_info = (RT_TASK_INFO*) rtdm_malloc(sizeof(RT_TASK_INFO));
	rt_task_inquire(task, task_info);
	HASH_FIND_STR(dtask_map, task_info->name, rtdtask);
	if (rtdtask){
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_stopwdg] stopping watchdog of task (%s)\n",task_info->name);
		#endif
		ret=0;//rt_alarm_stop( rtdtask->dtask.watchd );
	}
	rtdm_free(task_info);
	return ret;
}
/*
* Update the TASK_INFO structue of the associated RT_TASK
* Not really efficient at the moment.
*/
int rt_dtask_updateinfo ( RT_TASK * task ){
	int ret=-1;
	RT_DTASK * rtdtask;
	RT_TASK_INFO * task_info = (RT_TASK_INFO*) rtdm_malloc(sizeof(RT_TASK_INFO));
	rt_task_inquire(task, task_info);
	HASH_FIND_STR(dtask_map, task_info->name, rtdtask);
	if (rtdtask){
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@rt_updinfo] updating info of task (%s)\n",task_info->name);
		#endif
		ret=rt_task_inquire( rtdtask->dtask.task , &(rtdtask->dtask.task_info) );
	}
	rtdm_free(task_info);
	return ret;
}
/*
* Reset task's deadline with previous setted value.
*
* Note: might cause a priority recalculation
*/
int rt_dtask_resetdeadline( RT_TASK * task){
	int ret=-1;
	RT_DTASK * rtdtask;
	RT_TASK_INFO * task_info = (RT_TASK_INFO*) rtdm_malloc(sizeof(RT_TASK_INFO));
	rt_task_inquire(task, task_info);
	HASH_FIND_STR(dtask_map, task_info->name, rtdtask);
	if (rtdtask){
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_resetdline] resetting deadline of task (%s)\n",
			task_info->name);
		#endif
		rtdtask->dtask.deadline = rt_timer_read() + rtdtask->dtask.relative_deadline;
		rtdtask->dtask.remain=rtdtask->dtask.relative_deadline;
		rtdtask->dtask.status=OK;
		// TODO: reset watchdog
		//rt_alarm_stop( rtdtask->dtask.watchd );
		//rt_alarm_start( rtdtask->dtask.watchd, rtdtask->dtask.relative_deadline, TM_INFINITE);
		rt_dtask_recalculateprio();
		ret=0;
	}
	rtdm_free(task_info);
	return ret;
}
/*
* Set task's deadline. New deadline is meant to be a relative 
* deadline value (eg. task deadline is an 200000ns since it's start or modification)
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
*
* Note: might cause a priority recalculation
*/
int rt_dtask_setdeadline(RT_TASK * task, unsigned long newdeadline){
	int ret=-1;
	RT_DTASK * rtdtask;
	RT_TASK_INFO * task_info = (RT_TASK_INFO*) rtdm_malloc(sizeof(RT_TASK_INFO));
	rt_task_inquire(task, task_info);
	HASH_FIND_STR(dtask_map, task_info->name, rtdtask);
	if (rtdtask){
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_setdline] setting deadline of task(%s) to %lu\n",
										task_info->name, newdeadline);
		#endif
		rtdtask->dtask.relative_deadline = newdeadline;
		rtdtask->dtask.deadline = rt_timer_read() + rtdtask->dtask.relative_deadline;
		rtdtask->dtask.remain=rtdtask->dtask.relative_deadline;
		rtdtask->dtask.status=OK;
		// TODO: reset watchdog
		//rt_alarm_stop( rtdtask->dtask.watchd );
		//if (rtdtask->dtask.relative_deadline != DEADLINENOTSET)
		//	rt_alarm_start( rtdtask->dtask.watchd, rtdtask->dtask.relative_deadline, TM_INFINITE);
		rt_dtask_recalculateprio();
		ret=0;
	}
	return ret;
}
/*
* Create a Task with deadline specification.
* Deadline is relative and might be interpreted as the maximum task duration permitted.
* Deadline is expressed in nanosecs or in periods ( depends on how xenomai kernel has 
* been compiled, ONESHOT or PERIODIC mode respectively)
* Take a look at "A Tour of the Native API"
*
* NOTE: cause a priority recalculation;
*/
int rt_dtask_create ( RT_TASK * task, unsigned long deadline ){
	RT_DTASK * rtdtask,* tmp;
	//char a_name [TNAME_LEN+5];
	#ifdef MODULE
	rtdtask = (RT_DTASK*) rtdm_malloc(sizeof(RT_DTASK));	
	#else
	rtdtask = (RT_DTASK*) malloc(sizeof(RT_DTASK));	
	#endif
	init_dtask( &(rtdtask->dtask) );
	rtdtask->dtask.task = task;
	rtdtask->dtask.relative_deadline = deadline;
	rtdtask->dtask.deadline =  ((unsigned long) rt_timer_read()) + rtdtask->dtask.relative_deadline;
	rtdtask->dtask.remain=rtdtask->dtask.relative_deadline;
	rt_task_inquire( rtdtask->dtask.task , &(rtdtask->dtask.task_info) );
	HASH_FIND_STR( dtask_map, rtdtask->dtask.task_info.name, tmp);
	if (tmp==NULL){
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_create] creating dtask (%s) with deadline %lu\n",
			rtdtask->dtask.task_info.name, deadline);
		#endif
		//strncpy( (char*) &(a_name), (char*)&(rtdtask->dtask.task_info.name),TNAME_LEN);
		//a_name[TNAME_LEN] = "-WDG\0";
		//rt_alarm_create ( rtdtask->dtask.watchd, (char*) &(a_name), 
		//					&deadline_missed, (void*) rtdtask->dtask.task);
		//if (rtdtask->dtask.relative_deadline != DEADLINENOTSET)
		//	rt_alarm_start( rtdtask->dtask.watchd, rtdtask->dtask.relative_deadline, TM_INFINITE);
		HASH_ADD_STR( dtask_map, dtask.task_info.name , rtdtask);
	}else{
 		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_create] creation failed. Task (%s) already present\n",rtdtask->dtask.task_info.name);
		#endif
		rtdm_free(rtdtask);
		return -1;
	}
	rt_dtask_recalculateprio();
	return 0;
}
/*
* Delete a RT_TASK.
* This exclude the specified RT_TASK from the EDF based priority rearrangment.
* Latest setted priority are maintained so this procedure should be invoked specifying
* a DELETED task only.
*/
int rt_dtask_delete ( RT_TASK * task ){
	int ret=-1;
	RT_DTASK * rtdtask;
	RT_TASK_INFO * task_info= (RT_TASK_INFO*) rtdm_malloc(sizeof(RT_TASK_INFO));
	rt_task_inquire(task, task_info);
	HASH_FIND_STR(dtask_map, task_info->name, rtdtask);
	if (rtdtask){
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_delete] deleting dtask (%s)\n",task_info->name);
		#endif
		HASH_DEL(dtask_map, rtdtask );
		//rt_alarm_stop( rtdtask->dtask.watchd );
		clear_dtask( &(rtdtask->dtask) );
		rtdm_free(rtdtask);
		ret=0;
	}
	rtdm_free(task_info);
	return ret;
}
/*
* Initialize the data structures needed for handling EDF based priorities arrangement.
*/
int rt_dtask_init(void){
	dtask_map = NULL;
		#ifdef DEBUG
		rtdm_printk("Edfomai: [@dt_init] initializing dtask data structures\n");
		#endif
	return 0;
}
/*
* Free all data structures.
*/
int rt_dtask_dispose(void){
	RT_DTASK * rtdtask;
	RT_DTASK * tmp;
	#ifdef DEBUG
	rtdm_printk("Edfomai: [@dt_dispose] clearing dtask data structures\n");
	#endif
	HASH_ITER(hh, dtask_map, rtdtask, tmp) {
		HASH_DEL(dtask_map,rtdtask);
		//rt_alarm_stop( rtdtask->dtask.watchd );
		clear_dtask( &(rtdtask->dtask) );
		rtdm_free(rtdtask);
	}
	return 0;
}


