#include <native/task.h>

#define DEADLINENOTSET NULL

typedef enum TaskEvent {
	STARTED=0,
	SWITCHED=1,
	DELETED=2
} TaskEvent;

typedef enum EDFCommand {
	CREATE_TASK=0,  	
	START_TASK=1,
	RESET_DEADLINE=2,
	SET_DEADLINE=3
} EDFCommand;


typedef struct EDFMessage {
	EDFCommand command;
	RT_TASK task;
	RTIME deadline;
} EDFMessage;



EDFMessage * create_edfmessage( RT_TASK * task, RTIME deadline );
void delete_edfmessage( EDFMessage * message);

