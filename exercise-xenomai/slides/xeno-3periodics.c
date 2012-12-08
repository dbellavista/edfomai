RT_TASK demo_task; RT_SEM s;
int main (int argc, char * argv[]){
    char str[10]; int periods[3]; int i;
    rt_print_auto_init(1);
    mlockall(MCL_CURRENT|MCL_FUTURE);
    rt_printf("[Main]:start task\n");
    for(i=0;i<3;i++){
        sprintf(str,"Task-%d",i);
        periods[i]=i+1;
        rt_task_create(&demo_task,str,10,50+i,0);
        rt_task_start(&demo_task,&demo,&periods[i]);
    }
    rt_printf("end program by CTRL-C\n"); pause();
}

void demo(void *arg){
    RTIME p = 1e9; RT_TASK_INFO curtaskinfo;
    int retval = rt_task_inquire(rt_task_self(),
              &curtaskinfo); //inquire current task
    retval = * (int *)arg; p *= retval;
    rt_task_set_periodic(NULL,TM_NOW,p);
    while(1) {
        rt_timer_spin(p/2); // Busy Wait
        rt_printf("[%s] %d s periodic\n",
                        curtaskinfo.name,retval);
        rt_task_wait_period(NULL);
    }
}