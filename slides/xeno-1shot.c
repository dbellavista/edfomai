static RT_TASK t1, t2; RT_SEM semGlobal1, semGlobal2;
int global = 0;
int main (int argc, char * argv[]){
    // Init the process non-rt output buffer
    rt_print_auto_init(1);
    // Disable linux on-demand paging scheme
    mlockall(MCL_CURRENT|MCL_FUTURE);
    rt_sem_create(&semGlobal1,"semaphore1",1,S_FIFO);
    rt_sem_create(&semGlobal2,"semaphore2",0,S_FIFO);
    rt_task_create(&t1,"taskOne",0,10,0);
    rt_task_create(&t2,"taskTwo",0,10,0);
    rt_task_start(&t1,&taskOne,NULL);
    rt_task_start(&t2,&taskTwo,NULL);
}

void taskOne(void *arg){
    int i;
    for(i=0;i<ITER;i++){
        rt_sem_p(&semGlobal1,0);
        rt_printf("I'm taskOne and global = %d...\n",
                                            ++global);
        rt_sem_v(&semGlobal2);
    }
}

void taskTwo(void *arg){
    int i;
    for(i=0;i<ITER;i++){
        rt_sem_p(&semGlobal2,0);
        rt_printf("I'm taskTwo and global = %d---\n",
                                            --global);
        rt_sem_v(&semGlobal1);
    }
}