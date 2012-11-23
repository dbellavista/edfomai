#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
 
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
  
#include  <rtdk.h>
#define PLCperiod 8*1e9
//#define Initial cad

RT_TASK task;

RT_SEM readDone;
RT_SEM executionDone;
//RT_SEM writeDone;
short stopped = 0;

void reader(void *args){
	rt_task_set_periodic(NULL,TM_NOW,PLCperiod);
	while(!stopped){
		/*
		 *	Read inputs status
		*/
		rt_printf("sensing\n");
		sleep(4);
		rt_sem_v(&readDone);
		rt_printf("sensed\n");
		rt_task_wait_period(NULL);
	}
}

void writer(void *args){
	rt_task_set_periodic(NULL,TM_NOW,PLCperiod);
	while(!stopped){
		rt_sem_p(&executionDone,0);
		/*
		 *	Write outputs status
		 */
		rt_printf("acting\n");
		sleep(1);
		rt_printf("acted\n");
		rt_task_wait_period(NULL);
	}
}

void step0(){
}

void step1(){
}

void step2(){
}

void step3(){
}

void step4(){
}

void step5(){
}

void executor(void *args){
	rt_task_set_periodic(NULL,TM_NOW,PLCperiod);
	short step = 0;
	short actuators = 0;
	while(!stopped){
		rt_sem_p(&readDone,0);
		int sensors = 287265;
		switch(step){
			case 0:
				step0();
				if("condition")
					step = 1;
				break;
			case 1:
				step1();
				if("condition")
					step = 2;
				break;
			case 2:
				step2();
				if("condition")
					step = 3;
				break;
			case 3:
				step3();
				if("condition")
					step = 4;
				break;
			case 4:
				step4();
				if("condition")
					step = 5;
				break;
			case 5:
				step5();
				if("condition")
					step = 0;
				break;
		}
		rt_printf("executing\n");
		sleep(2);
		rt_sem_v(&executionDone);
		rt_printf("executed\n");
		rt_task_wait_period(NULL);
	}
	return;
}

void init_xenomai() {
	// Perform auto-init of rt_print buffers if the task doesn't do so
	rt_print_auto_init(1);

 	// Avoids memory swapping for this program
	mlockall(MCL_CURRENT|MCL_FUTURE);
}

void startup(){
	rt_sem_create(&readDone, "readSem", 0, S_PRIO);
	rt_sem_create(&executionDone, "executionSem", 0, S_PRIO);
	//rt_sem_create(&writeDone, "writeSem", 0, S_PRIO);
	
	rt_task_create(&task, "readerTask", 0, 99, 0);
	rt_task_start(&task, &reader, NULL);

	rt_task_create(&task, "executorTask", 0, 98, 0);
	rt_task_start(&task, &executor, NULL);

	rt_task_create(&task, "writerTask", 0, 97, 0);
	rt_task_start(&task, &writer, NULL);
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
}

int main(int argn, char** argv){
	init_xenomai();
	rt_printf("initialized.\n");
	rt_printf("starting up...\n");
	startup();
	rt_printf("waiting Ctrl+C\n");
	wait_for_ctrl_c();
	rt_printf("closing...\n");
	cleanup();
	return 0;
}
