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
	unsigned long* params;
	int i;
	retval = rt_task_inquire(curtask,&curtaskinfo);
	if(retval<0){
		rt_printf("inquiring error %d:%s\n",-retval,strerror(-retval));
	}
	//print task name
	params = (int *)arg;
	RTIME p = 1e8; // 0.1 secs
	p*=params[0];
	rt_task_set_periodic(NULL,TM_NOW,p);
	while(1){
		//for(i=0;i<params[1]*10000;i++)
		//	if(!(i%1000))
		//		rt_printf("[%s] %d s, %d\n", curtaskinfo.name,param[0],param[1]);
		rt_timer_spin(param[1]*1e8);
		rt_task_wait_period(NULL);
	}
}

int main (int argc, char * argv[]){
	char str[10];
	unsigned long periods[3][3];
	int i;
	rt_print_auto_init(1);
	mlockall(MCL_CURRENT|MCL_FUTURE);
	edf_init();

	periods[0][0]=8;	// task0	period
	periods[0][1]=2;	//		timeUsed
	periods[0][2]=8;	//		deadline
	periods[1][0]=16;	// task1	period
	periods[1][1]=3;	// 		...
	periods[1][2]=16;
	periods[2][0]=12;
	periods[2][1]=5;
	periods[2][2]=12;

	rt_printf("[Main]:start task\n");
	for(i=0;i<3;i++){
		sprintf(str,"Task-%d",i);
		periods[i]=i+1;
		rt_task_create(&demo_task,str,10,50,0);
		edf_task_start(&demo_task,periods[2],&demo,periods[i]);
	}
	rt_printf("end program by CTRL-C\n");
	pause();
}
