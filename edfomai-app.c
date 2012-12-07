#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtdm/rtdm.h>
#include <native/mutex.h>
#include "edfomai-app.h"
#include <rtdk.h>
#define DEBUG
#define DEF_PRIO 20
#define DEVICE_NAME "edfomai"
#define EDFOMAI_MTXNAME "edfomai-mtx"
/*
* EDF-Scheduler device file-descriptor
*/
int device=-1;
RT_MUTEX edfomai_mutex;

int _mtx_create(void);
int _mtx_delete(void);
int _mtx_acquire(void);
int _mtx_release(void);

/*
* Initialize EDFOMAI resources - Not thread safe
*/
int edf_init() {
	int ret;
	device=-1;
	//if (device!=-1)
	//	return 0;
	ret=_mtx_create();
	if (ret) 
		return -1;
	// EEXIST and 0 are welcome ..
	device=rt_dev_open(DEVICE_NAME, 0);
	if (device < 0) {
		rt_printf("Edfomai: can't open device %s (%s)\n", DEVICE_NAME, 
			strerror(-device));
		device=-1;
		return -1;
		}
	return 0;
}
/*
* Dispose EDFOMAI resources
*/
int edf_dispose(){
	int ret;
	ret=_mtx_acquire();

	if (ret||device==-1){
		rt_printf("Edfomai: edfomai may not be initialized. You cant dispose it yet!\n");
		return -1;
	}
	ret=_mtx_delete();	
	ret=rt_dev_close(device);
	if (ret){
		#ifdef DEBUG
			rt_printf("Edfomai: can't close device \"%s\" (%s)\n", 
			DEVICE_NAME, strerror(-ret));
		#endif
		device=-1;
			return -2;
		}
	return 0;
}
int _mtx_create(){
	int ret=0;
	//ret=rt_mutex_create( &edfomai_mutex , EDFOMAI_MTXNAME );
	if (-ret==ENOMEM){
		rt_printf("Edfomai: not enought memory (%s)\n" , 
		strerror(-ret) );
	}
	if (-ret==EPERM){
		rt_printf("Edfomai: task cant create mutex in this context (%s)\n" , strerror(-ret) );	
	}
	return ret;
}
int _mtx_delete(){
	int ret=0;
	//ret=rt_mutex_delete( &edfomai_mutex );
	return ret;
}
/*
* 
*/
int _mtx_acquire(){
	int ret=0;
	//ret=rt_mutex_acquire( &edfomai_mutex, TM_INFINITE);
	if (-ret==EINVAL){
		rt_printf("Edfomai: not a valid mutex (%s)\n" , strerror(-ret) );
	}
	if (-ret==EWOULDBLOCK){
		rt_printf("Edfomai: mutex locked by someone else (%s)\n" , strerror(-ret) );
	}
	if (-ret==ETIMEDOUT){
		rt_printf("Edfomai: mutex acquire timed out (%s)\n" , strerror(-ret) );
	}
	if (-ret==EPERM){
		rt_printf("Edfomai: cant acquire mutex in this context (%s)\n",strerror(-ret) );
	}
	if (-ret==EINTR){
		rt_printf("Edfomai: someone has interrupted the waiting (%s)\n",strerror(-ret));
	}
	
	return ret;
}
/*
* 
*/
int _mtx_release(){
	int ret=0;
	//ret=rt_mutex_release( &edfomai_mutex);
	
	if (-ret==EINVAL){
		rt_printf("Edfomai: not a valid mutex (%s)\n" , strerror(-ret) );
	}
	if (-ret==EIDRM){
		rt_printf("Edfomai: mutex descriptor in use has been deleted (%s)\n" , strerror(-ret) );
	}
	if (-ret==EPERM){
		rt_printf("Edfomai: cant acquire mutex in this context (%s)\n",strerror(-ret) );
	}
	
	return ret;
}
/*
* 
*/
int _wait_period(unsigned long * overruns){
	int ret=0;
	ret=rt_task_wait_period( overruns );
	if (-ret==EWOULDBLOCK){
		rt_printf("Edfomai: task is not periodic (%s)\n" , strerror(-ret) );
		ret=-1;
	}else if (-ret==EINTR){
			rt_printf("Edfomai: task has been interrupted (%s)\n" , strerror(-ret) );
		ret=-1;
		}else if (-ret==ETIMEDOUT){
			rt_printf("Edfomai: task has waited too long (%s)\n" , strerror(-ret) );
		ret=-1;
		}else if (-ret==EPERM){
			rt_printf("Edfomai: task cant sleep in this context (%s)\n" , strerror(-ret) );
		}else ret=0;
	return ret;
}
/*
* Start a RT Task specifying a deadline.
* Deadline is meant as a relative timeout in nanose or in periods 
* (depends on how you have compiled xenomai kernel patch). Otherwise can be seedt to 
* DEADLINENOTSET .
*/
int edf_task_start( RT_TASK * task, unsigned long long deadline, void(*procedure)(void *arg), void * arg ){
	int ret;
	EDFMessage * message = (EDFMessage*) malloc(sizeof(EDFMessage));
	RT_TASK_INFO task_info;
	
	ret=rt_task_inquire(task, &task_info);
	if (ret){
		rt_printf("Edfomai: cant retrieve task info for specified task\n");
		free(message);
		return -1;
	}

	message->command = CREATE_TASK;
	message->deadline = deadline;
	ret=(int) strncpy( (char*)&(message->task), (char*)&(task_info.name), TNAME_LEN );
	message->task[TNAME_LEN]=0; // null terminate the string 
	if ( ret != (int) &(message->task)){
		rt_printf("Edfomai: something wrong during a copy (%s)\n", strerror(-ret) );
		free(message);
		return -1;
	}
	
	ret=_mtx_acquire();	
	if ( ret!= 0 || device == -1 ){
		rt_printf("Edfomai: you should init edfomai first\n");
		
		free(message);
		return -1;
	}
	else{
		ret=rt_dev_write(device, (const void *) message, sizeof(EDFMessage));
		ret=0;
	}
	_mtx_release();

	ret=rt_task_start(task, procedure, arg);

	free(message);
	return ret;
}
/*
* Wait a period and reset the RT Task deadline.
* Once awaken, rt task deadline will be resetted to the initial specified value. 
*/
int edf_task_wait_period(unsigned long * overruns){
	int ret, ret1;
	EDFMessage * message = (EDFMessage*) malloc(sizeof(EDFMessage));
	RT_TASK * currtask;
	RT_TASK_INFO task_info;

	currtask = rt_task_self();
	if (!currtask){
		rt_printf("Edfomai: [@waitperiod] cant retrieve task, are we in asynch context??\n");
		free(message);
		return -1;
	}
	ret=rt_task_inquire(currtask, &task_info);
	if (ret){
		rt_printf("Edfomai: [@waitperiod] cant retrieve task info for current task\n");
		free(message);
		return -1;
	}
	message->command=STOP_WATCHD;
	message->deadline=DEADLINENOTSET;
	ret = (int) strncpy( (char*) &(message->task), (char*)&(task_info.name), TNAME_LEN );
	message->task[TNAME_LEN]=0;
	if ( ret!=(int) &(message->task)){
		rt_printf("Edfomai: [@waitperiod] something wrong with memcpy (%s)\n",strerror(-ret));
		free(message);
		return -1;
	}
	ret1=rt_dev_write(device,(const void *) message, sizeof(EDFMessage));
	#ifdef DEBUG
	rt_printf("Edfomai: [@waitperiod] task(%s) wait next period\n", task_info.name,ret);
	#endif
	ret = _wait_period(overruns);
	#ifdef DEBUG
	rt_printf("Edfomai: [@waitperiod] task(%s) awaken ret(%d)\n", task_info.name,ret);
	#endif
	ret=_mtx_acquire();
	if (ret){
		free(message);
		return -1;
	}
	if (device==-1){
		rt_printf("Edfomai: you should init edfomai first\n");
		ret1=-1;
	}
	else{
		#ifdef DEBUG
		rt_printf("Edfomai: [@waitperiod] sending reset request\n");
		#endif
		message->command=RESET_DEADLINE;
		ret1=rt_dev_write(device,(const void *) message, sizeof(EDFMessage));
	}
	ret = _mtx_release();
	if (ret || ret1)
		ret=-1;
	else
		ret=0;
	free(message);
	return ret;
}

