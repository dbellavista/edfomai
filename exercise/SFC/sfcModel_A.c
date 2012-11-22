#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
 
#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>
  
#include  <rtdk.h>

#include "program.h"


RT_TASK task;

RT_SEM readDone;
RT_SEM executionDone;

short stopped = 0;
void* sensors;
void* actuators;

void sense(void *args);
void execute(void *args);
void act(void *agrs);

void sense(void *args){
	rt_task_set_periodic(NULL,TM_NOW,PLCperiod);
	while(!stopped){
		/*
		 *	Read inputs status
		*/
		sensors = readInputs();
		
		rt_sem_v(&readDone);
//		rt_printf("sensed\n");
		rt_task_wait_period(NULL);
	}
}

void act(void *args){
	rt_task_set_periodic(NULL,TM_NOW,PLCperiod);
	while(!stopped){
		rt_sem_p(&executionDone,0);
		/*
		 *	Write outputs status
		 */
		writeOutputs(actuators);
//		rt_printf("acted\n");
		rt_task_wait_period(NULL);
	}
}

void executor(void *args){
	rt_task_set_periodic(NULL,TM_NOW,PLCperiod);
	short step = (short)*args;
	short actuators = 0;
	while(!stopped){
		rt_sem_p(&readDone,0);
		int sensors = 287265;
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
