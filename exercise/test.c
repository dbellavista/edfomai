#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>

#include <rtdk.h>
RT_TASK demo_task;

void demo (void *arg){
	RT_TASK *curtask;
	RT_TASK_INFO curtaskinfo;

	//hello world
	rt_printf("Hello World!\n");

	//inquire current task
	curtask = rt_task_self();
	rt_task_inquire(curtask,&curtaskinfo);

	//print task name
	rt_printf("Task name: %s \n", curtaskinfo.name);
}

int main (int argc, char * argv[]){
	char str[10];
	rt_print_auto_init(1);
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_printf("start task\n");
	sprintf(str,"hello");
	rt_task_create(&demo_task,str,0,50,0);
	rt_task_start(&demo_task,&demo,0);
}

