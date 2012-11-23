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
	//inquire current task
	curtask = rt_task_self();
	int retval;
	retval = rt_task_inquire(curtask,&curtaskinfo);
	if(retval<0){
		rt_printf("inquiring error %d:%s\n",-retval,strerror(-retval));
		return;
	}
	//print task name
	retval = * (int *)arg;
	rt_printf("Task name: %s. Has param: %d\n", curtaskinfo.name,retval);
}

int main (int argc, char * argv[]){
	char str[10];
	int i;
	rt_print_auto_init(1);
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_printf("[Main]:start task\n");
	for(i=0;i<5;i++){
		sprintf(str,"Task-%d",i);
		rt_task_create(&demo_task,str,10,50,0);
		rt_task_start(&demo_task,&demo,&i);
	}
}

