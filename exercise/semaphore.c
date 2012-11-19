#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>
#include <native/sem.h>

#include <rtdk.h>

#define ITER 10

static RT_TASK t1;
static RT_TASK t2;

RT_SEM semGlobal1;
RT_SEM semGlobal2;

int global = 0;

void demo (void *arg){
	sleep(1);
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;
	//inquire current task
	curtask = rt_task_self();
	int retval;
	retval = rt_task_inquire(curtask,&curtaskinfo);
	if(retval<0){
		rt_printf("inquiring error %d:%s\n",-retval,strerror(-retval));
	}
	//print task name
	retval = * (int *)arg;
	RTIME p = 1e9;
	p*=retval;
	rt_task_set_periodic(NULL,TM_NOW,p);
	while(1){
		rt_printf("[%s] %d s periodic\n", curtaskinfo.name,retval);
		rt_task_wait_period(NULL);
	}
}


void taskOne(void *arg){
	int i;
	for(i=0;i<ITER;i++){
		rt_sem_p(&semGlobal1,0);
		rt_printf("I'm taskOne and global = %d...\n",++global);
		rt_sem_v(&semGlobal2);
	}
}

void taskTwo(void *arg){
	int i;
	for(i=0;i<ITER;i++){
		rt_sem_p(&semGlobal2,0);
		rt_printf("I'm taskTwo and global = %d---\n",--global);
		rt_sem_v(&semGlobal1);
	}
}

int main (int argc, char * argv[]){
	rt_print_auto_init(1);
	mlockall(MCL_CURRENT|MCL_FUTURE);
	
	rt_sem_create(&semGlobal1,"semaphore1",1,S_FIFO);
	rt_sem_create(&semGlobal2,"semaphore2",0,S_FIFO);
	
	rt_task_create(&t1,"taskOne",0,10,0);
	rt_task_create(&t2,"taskTwo",0,10,0);
	
	rt_task_start(&t1,&taskOne,NULL);
	rt_task_start(&t2,&taskTwo,NULL);
	
	//rt_printf("end program by CTRL-C\n");
	//pause();
}

