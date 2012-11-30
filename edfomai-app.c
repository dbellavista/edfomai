#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtdm/rtdm.h>
#include <native/mutex.h>
#include "edfomai-app.h"

#define DEF_PRIO 20
#define DEVICE_NAME "edfomai"
#define EDFOMAI_MTXNAME "edfomai-mtx"
/*
* EDF-Scheduler device file-descriptor
*/
static int device=-1;

static RT_MUTEX edfomai_mutex;




int _mtx_acquire( RTIME timeout);
int _mtx_release();

/*
* Initialize EDFOMAI resources - Not thread safe
*/
int init_edfomai() {
	int ret;
	if (device != -1)
		return 0;

	ret=rt_mutex_create( &edfomai_mutex , EDFOMAI_MTXNAME );

	if (-ret==ENOMEM){
                printf("Edfomai: not enought memory (%s)\n" , strerror(-ret) );
                return ret;
        }
        if (-ret==EPERM){
                printf("Edfomai: task cant create mutex in this context (%s)\n" , strerror(-ret) );
                return ret;
        }
	// EEXIST and 0 are welcome ..
	device=rt_dev_open(DEVICE_NAME, 0);

	if (device < 0) {
		printf("Edfomai: can't open device %s (%s)\n", DEVICE_NAME, 
			strerror(-device));
		return -1;
    	}
	return 0;
}
/*
* Dispose EDFOMAI resources
*/
int dispose_edfomai(){
	int ret;
	ret = _mtx_acquire(TM_INFINITE);

	if ( ret!=0 || device == -1){
		printf("Edfomai: edfomai may not be initialized. You cant dispose it yet!\n");
		return -1;
	}

	ret=rt_mutex_delete( &edfomai_mutex );	
	ret=rt_dev_close(device);

	if (ret < 0){
      		printf("Edfomai: can't close device %s (%s)\n", 
			DEVICE_NAME, strerror(-ret));
      		return 1;
    	}
  	return 0;
}

/*
* 
*/
int _mtx_acquire( RTIME timeout ){
	int ret;
	ret=rt_mutex_acquire( &edfomai_mutex, timeout);

        if (-ret==EINVAL){
                printf("Edfomai: not a valid mutex (%s)\n" , strerror(-ret) );
        }
        if (-ret==EWOULDBLOCK){
                printf("WARNING : mutex locked by someone else (%s)\n" , strerror(-ret) );
        }
        if (-ret==ETIMEDOUT){
                printf("WARNING : mutex acquire timed out (%s)\n" , strerror(-ret) );
        }
        if (-ret==EPERM){
                printf("Edfomai: cant acquire mutex in this context (%s)\n",strerror(-ret) );
        }
        if (-ret==EINTR){
                printf("WARNING : someone has interrupted the waiting (%s)\n",strerror(-ret));
        }
	return ret;
}
/*
* 
*/
int _mtx_release(){
        int ret;
	ret=rt_mutex_release( &edfomai_mutex);

        if (-ret==EINVAL){
                printf("Edfomai: not a valid mutex (%s)\n" , strerror(-ret) );
        }
        if (-ret==EIDRM){
                printf("Edfomai: mutex descriptor in use has been deleted (%s)\n" , strerror(-ret) );
        }
        if (-ret==EPERM){
                printf("Edfomai: cant acquire mutex in this context (%s)\n",strerror(-ret) );
        }
	return ret;
}
/*
* 
*/
int _wait_period( unsigned long * overruns_r ){
	int ret;
	ret = rt_task_wait_period( overruns_r );

	if (-ret==EWOULDBLOCK){
		printf("Edfomai: task is not periodic (%s)\n" , strerror(-ret) );
	}
        if (-ret==EINTR){
                printf("Edfomai: task has been interrupted (%s)\n" , strerror(-ret) );
        }
        if (-ret==ETIMEDOUT){
                printf("Edfomai: task has waited too long (%s)\n" , strerror(-ret) );
        }
        if (-ret==EPERM){
                printf("Edfomai: task cant sleep in this context (%s)\n" , strerror(-ret) );
        }
	return ret;
}
/*
* Start a RT Task specifying a deadline.
* Deadline is meant as a relative timeout in nanose or in periods 
* (depends on how you have compiled xenomai kernel patch). Otherwise can be seedt to 
* DEADLINENOTSET .
*/
int edf_task_start( RT_TASK * task, unsigned long deadline, void(*procedure)(void *arg), void * arg ){
	int ret;
	EDFMessage * message = (EDFMessage*) malloc(sizeof(EDFMessage));
	RT_TASK_INFO task_info;
	 
        ret=rt_task_inquire(task, &task_info);
        if (ret){
                printf("Edfomai: cant retrieve task info for specified task\n");
                free(message);
                return -1;
        }

	message->command = CREATE_TASK;
	message->deadline = deadline;
	ret=(int) strncpy( (char*)&(message->task), (char*)&(task_info.name), TNAME_LEN );
	message->task[TNAME_LEN]=0; // null terminate the string 
	if ( ret != (int) &(message->task)){
		printf("Edfomai: something wrong during a copy (%s)\n", strerror(-ret) );
		free(message);
		return -1;
	}
	
	ret=_mtx_acquire( TM_INFINITE );	
	if ( ret!= 0 || device == -1 ){
		printf("Edfomai: you should init edfomai first\n");
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
int edf_task_wait_period( unsigned long * overruns_r ){
	int ret, ret1;
	EDFMessage * message = (EDFMessage*) malloc(sizeof(EDFMessage));
	RT_TASK * currtask;
	RT_TASK_INFO task_info;

	message->command=RESET_DEADLINE;
	message->deadline=DEADLINENOTSET;
	currtask = rt_task_self();
	if (currtask == NULL){
		printf("Edfomai: cant retrieve RT_TASK descriptor, are we in asynch context??\n");
		free(message);
		return -1;
	}
	ret=rt_task_inquire(currtask, &task_info);
	if (ret){
		printf("Edfomai: cant retrieve task info for current task\n");
		free(message);
		return -1;
	}	

	ret = (int) strncpy( (char*) &(message->task), (char*)&(task_info.name), TNAME_LEN );
	message->task[TNAME_LEN]=0;
        if ( ret != (int) &(message->task)){
                printf("Edfomai: something wrong with memcpy (%s)\n",strerror(-ret) );
                free(message);
		return -1;
        }
	
	ret = _wait_period(overruns_r);
	if (ret){
		free(message);
		return -1;
	}
	ret = _mtx_acquire( TM_INFINITE );
	if (ret){
		free(message);
		return -1;
	}
 	if (device == -1 ){
                printf("Edfomai: you should init edfomai first\n");
		ret1=-1;
        }
	else{
        	ret1=rt_dev_write( device,(const void * ) &message, sizeof(EDFMessage) );
	}
	ret = _mtx_release();
	if (ret || ret1)
		ret=-1;
	else
		ret=0;
	free(message);
	return ret;
}

