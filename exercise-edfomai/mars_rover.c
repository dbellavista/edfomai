#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
#include <rtdk.h>

#include "../edfomai-app.h"

#define timeUnit 10e8//10000000 // 10 000 000 ns
RTIME t;

RT_TASK task;
RT_TASK spawner;

RT_SEM sensingDone;
RT_SEM processingDone;
RT_SEM exLock;
short stopped = 0;

int sharedResource;
int shared = 0;

unsigned long params[3][3];

void comunicate(void *arg){
	int res;
	rt_task_set_periodic(NULL,TM_NOW,66*t);
	while(!stopped){
		rt_sem_p(&exLock,0);
		res = sharedResource;
		rt_sem_v(&exLock);
		rt_printf("[comunicator] sending: %d\n",res);
		rt_timer_spin(6*t);
		rt_printf("[comunicator] sent: %d\n",res);
		edf_task_wait_period(NULL);
	}
}

//void (*process)(void *arg);
//void (*act)(void *arg);

void sense (void *arg){
	int i;
	int res;
	unsigned long* param;
	param = (unsigned long*)arg;
        RT_TASK *curtask;
        RT_TASK_INFO curtaskinfo;

	curtask = rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
        
	rt_task_set_periodic(NULL,TM_NOW,param[0]*t);

	rt_printf("[%s] started\n", curtaskinfo.name);

	while(!stopped){
		for ( i=0 ; i < param[1]; i++){
			rt_timer_spin(t);
			rt_printf("[%s] executing\n",curtaskinfo.name);
		}
		
		rt_sem_p(&exLock,0);
		sharedResource=shared;
		rt_sem_v(&exLock);
		
		rt_sem_v(&sensingDone);

		edf_task_wait_period(NULL);
	}
}

void process(void *arg){
	int i;
	unsigned long* param;
	param = (unsigned long*)arg;
        RT_TASK *curtask;
        RT_TASK_INFO curtaskinfo;

	curtask = rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
        
	rt_task_set_periodic(NULL,TM_NOW,param[0]*t);

	rt_printf("[%s] started\n", curtaskinfo.name);

	while(!stopped){
		rt_sem_p(&sensingDone,0);
		
		for ( i=0 ; i < param[1]; i++){
			rt_timer_spin(t);
			rt_printf("[%s] executing\n",curtaskinfo.name);
		}
		
		rt_sem_p(&exLock,0);
		sharedResource+=1;
		rt_sem_v(&exLock);
		
		rt_sem_v(&processingDone);

		edf_task_wait_period(NULL);
	}
}

void act(void *arg){
	int i;
	int res;
	unsigned long* param;
	param = (unsigned long*)arg;
        RT_TASK *curtask;
        RT_TASK_INFO curtaskinfo;

	curtask = rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);
        
	rt_task_set_periodic(NULL,TM_NOW,param[0]*t);

	rt_printf("[%s] started\n", curtaskinfo.name);

	while(!stopped){
		rt_sem_p(&processingDone,0);

		rt_sem_p(&exLock,0);
		res=sharedResource;
		rt_sem_v(&exLock);
		for ( i=0 ; i < param[1]; i++){
			rt_timer_spin(t);
			rt_printf("[%s] executing\n",curtaskinfo.name);
		}
		shared=res;

		edf_task_wait_period(NULL);
	}
}

void startup(){
	
	params[0][0]=11;	// t0	period
	params[0][1]=2;		//	busy
	params[0][2]=3;		//	deadline
	
	params[1][0]=11;	// t1	period
	params[1][1]=3;		//	busy
	params[1][2]=7;		//	deadline
	
	params[2][0]=11;	// t2	period
	params[2][1]=2;		//	busy
	params[2][2]=10;	//	deadline
	
//	act=process=sense;
	
	t = timeUnit;

	rt_sem_create(&sensingDone, "sensorSem", 0, S_PRIO);
	rt_sem_create(&processingDone, "processorSem", 0, S_PRIO);
	rt_sem_create(&exLock, "sharedResource", 1, S_PRIO);
	
	rt_task_create(&task, "sensor", 0, 90, 0);
	edf_task_start(&task, params[0][2]*t, &sense, params[0]);

	rt_task_create(&task, "processor", 0, 89, 0);
	edf_task_start(&task, params[1][2]*t, &process, params[1]);

	rt_task_create(&task, "actuator", 0, 88, 0);
	edf_task_start(&task, params[2][2]*t, &act, params[2]);

	rt_task_create(&task, "comunicator", 0, 87, 0);
	edf_task_start(&task, 65*t, &comunicate, NULL);
}

int init_edfomai() {
	// Perform auto-init of rt_print buffers if the task doesn't do so
	rt_print_auto_init(1);

 	// Avoids memory swapping for this program
	mlockall(MCL_CURRENT|MCL_FUTURE);

	// Initialize edf
	return edf_init();

//	// Make the current task an RT_TASK
//	rt_task_shadow(&spawner, str0,50,T_CPU(0));
}

void catch_signal(int sig) {
	stopped=1;
	rt_printf("--should I exit?--\n");
	//exit(0);
}

void wait_for_ctrl_c() {
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	// wait for SIGINT (CTRL-C) or SIGTERM signal
	pause();
}

void cleanup(){
	rt_printf("cleaned\n");
	edf_dispose();
}

int main(int argn, char** argv){
	if (init_edfomai())
		return -1;
	rt_printf("initialized.\n");
	rt_printf("starting up...\n");
	startup();
	rt_printf("waiting Ctrl+C\n");
	wait_for_ctrl_c();
	rt_printf("closing...\n");
	cleanup();
	return 0;
}
