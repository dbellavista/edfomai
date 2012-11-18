#include <rtdm/rtdm_driver.h>
#include <native/task.h>
#include <native/timer.h>
#include "edfomai-data.h"
// The mutex with this name SHOULD be created and registered by the kernel module.
#define DMUTEX_NAME "EDF_DATA_MUTEX"

struct rt_deadline_task {
	nanosecs_abs_t deadline;
	nanosecs_abs_t remain;
	RT_TASK task;
	RT_TASK_INFO task_info;
} RT_DEADLINE_TASK;

int rt_dtask_recalculateprio();
int rt_dtask_updateinfo ( RT_TASK * task );
int rt_dtask_resetdeadline( RT_TASK * task);
int rt_dtask_setdeadline(RT_TASK * task, RTIME newdeadline);
int rt_dtask_create ( RT_TASK * task, RTIME deadline );
int rt_dtask_delete ( RT_TASK * task );

int rt_dtask_init();
int rt_dtask_dispose();


