#ifndef _EDFOMAI_DATA_
#define _EDFOMAI_DATA_

#ifdef MODULE
#include <native/task.h>
#else
#include <native/task.h>
#endif


#define DEADLINENOTSET -1

/*
* Command Type
*/
typedef enum EDFCommand {
	CREATE_TASK=0,  	
	START_TASK=1,
	RESET_DEADLINE=2,
	SET_DEADLINE=3
} EDFCommand;
/*
* Message interpretable by the edfomai driver module
*/
typedef struct EDFMessage {
	EDFCommand command;
	RT_TASK task;
	unsigned long deadline;
} EDFMessage;

#endif
