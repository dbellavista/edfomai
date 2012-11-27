#include "edfomai-drv-data.h"

#define NTESTS 6
#define FAILED 1
#define PASSED 0

RT_TASK * t1,t2,t3,t4,t5;
unsigned long d1,d2,d3,d4,d5;

void _populate(){
return;
}

int test_recalculateprio(){
return FAILED;
}
int test_updateinfo (){
return FAILED;
}
int test_resetdeadline(){
return FAILED;
}
int test_setdeadline(){
return FAILED;
}
int test_create (){
return FAILED;
}
int test_delete (){
return FAILED;
}



int main (){
	int r[NTESTS];
	int result=0, i=0;
	printf("###########################\n");
	printf("### DATA STRUCTURE TESTING \n");
	printf("###########################\n");

	rt_dtask_init();
	_populate();
	r[0]=test_create();
	rt_dtask_dispose();
	
	rt_dtask_init();
	_populate();
	r[1]=test_delete();
	rt_dtask_dispose();

	rt_dtask_init();
	_populate();
	r[2]=test_resetdeadline();
	rt_dtask_dispose();
	
	rt_dtask_init();
	_populate();
	r[3]=test_recalculateprio();
	rt_dtask_dispose();
	
	rt_dtask_init();
	_populate();
	r[4]=test_updateinfo();
	rt_dtask_dispose();

	rt_dtask_init();
	_populate();
	r[5]=test_setdeadline();
	rt_dtask_dispose();
	for (i=0; i<NTESTS; i++)
		result+=r[i];

	if (result==0){
 		printf("DATA STRUCTURE TEST OK (%d/%d)\n", (NTESTS-result), NTESTS );
	}else{
		printf("DATA STRUCTURE TEST FAILED (%d/%d)\n", (NTESTS-result), NTESTS );
	}
	return result;
}
