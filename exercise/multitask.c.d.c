#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>

#include <native/task.h>
#include <native/timer.h>

#include <rtdk.h>
RT_TASK demo_task;

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

int main (int argc, char * argv[]){
	char str[10];
	int periods[3];
	int i;
	rt_print_auto_init(1);
	mlockall(MCL_CURRENT|MCL_FUTURE);
	rt_printf("[Main]:start task\n");
	for(i=0;i<3;i++){
		sprintf(str,"Task-%d",i);
		periods[i]=i+1;
		rt_task_create(&demo_task,str,10,50,0);
		rt_task_start(&demo_task,&demo,&periods[i]);
	}
	rt_printf("end program by CTRL-C\n");
	pause();
}

