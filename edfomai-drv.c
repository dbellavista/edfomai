#include <linux/module.h>
#include <native/mutex.h>
#include "edfomai-data.h"
#include "edfomai-drv-data.h"

#define DEVICE_NAME "edfomai"
#define SOME_SUB_CLASS 4711
#define POLL_DELAY 1000

/**
* The context of a device instance
* A context is created each time a device is opened and passed to
* other device handlers when they are called.
*/
typedef struct context_s{
} context_t;

/* 
* Access to edf data structures and services must be coordinated through this mutex
*/
static RT_MUTEX edf_data_mutex;

/**
* Open the device
* This function is called when the device shall be opened.
*/
static int edf_rtdm_open_nrt(struct rtdm_dev_context *context, rtdm_user_info_t *user_info, int oflags)
{
	return 0;
}

/**
* Close the device
* This function is called when the device shall be closed.
*/
static int edf_rtdm_close_nrt(struct rtdm_dev_context *context, rtdm_user_info_t * user_info)
{
	return 0;
}

/**
* Read from the device
* This function is called when the device is read (non-realtime)
* context.
*/
static ssize_t edf_rtdm_read_nrt(struct rtdm_dev_context *context, rtdm_user_info_t * user_info, void *buf, size_t nbyte)
{
  // TODO: copy to user some kind of useful information for monitoring purposes..
  return -1;
}

/**
* Write in the device
* This function is called when the device is written (non-realtime).
* NOTE: only EDFMessage are interpreted by this routine, a negative value wille be
* returned in case of error.
*/
static ssize_t edf_rtdm_write_nrt(struct rtdm_dev_context *context, rtdm_user_info_t * user_info, const void *buf, size_t nbyte)
{
	EDFMessage * message;
	int status;
	
	if (nbyte!=sizeof(struct EDFMessage))
	      return -1; /* You should talk to EDF-SCHED with EDFMessages */
	
	message = (EDFMessage*) rtdm_malloc(sizeof(EDFMessage));
	if ( rtdm_copy_from_user( user_info, message, buf, sizeof(EDFMessage) ) == EFAULT ){
		rtdm_free(message);
		return EFAULT;}
	
	switch (message->command)
	{
	      case CREATE_TASK:
	              status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
	              if (status){
	                      rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
				rtdm_free(message);
				return status;
	              }
	              rt_dtask_create ( &(message->task) , message->deadline );
	              /* 
	      	* Task should be started outside the module 
	      	* because function pointer to new task main procedure 
	      	* must be provided during the rt_task_start.
	      	* We haven't it here.
	      	*/
	              status =rt_mutex_release(&edf_data_mutex);
	              if (status){
	                      rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
				rtdm_free(message);
				return status;
	              }
	
	      break; 
	      case START_TASK:
	      	/* 
	      	* We only set the task deadline. 
	      	* If you want to use this functionality properly you should
	      	* first create the rt_task specifying the DEADLINENOTSET param, 
	      	* then set the actual deadline using this command. 
	      	* This way task deadline will not be considered (more or less:).
	      	*/
	      	status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
	      	if (status){
	      		rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
	      		rtdm_free(message);
			return status;
	      	}
	      	rt_dtask_setdeadline ( &(message->task) , message->deadline );
	              status =rt_mutex_release(&edf_data_mutex);
	              if (status){
	                      rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
	                	rtdm_free(message);  
			    	return status;
	              }
	      break;		
	      case RESET_DEADLINE:
	      	status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
	      	if (status){
	      		rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
	      		rtdm_free(message);
			return status;
	      	} 
	              rt_dtask_resetdeadline ( &(message->task) );
	              status =rt_mutex_release(&edf_data_mutex);
	              if (status){
	                      rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
	                      	rtdm_free(message);
				return status;
	              }
	      break;
              case SET_DEADLINE:
                status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
                if (status){
                        rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
                        rtdm_free(message);
			return status;
                }
                      rt_dtask_setdeadline ( &(message->task) , message->deadline);
                      status =rt_mutex_release(&edf_data_mutex);
                      if (status){
                              rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
                              	rtdm_free(message);
				return status;
                      }
              break;

	}
	rtdm_free(message);
	return nbyte;
}



/*
* This structure describe the simple RTDM device
*/
static struct rtdm_device device = {
	.struct_version = RTDM_DEVICE_STRUCT_VER,
	.device_flags = RTDM_NAMED_DEVICE,
	.context_size = sizeof(context_t),
	.device_name = DEVICE_NAME,
	.open_nrt = edf_rtdm_open_nrt,
	.ops = {
		.close_nrt = edf_rtdm_close_nrt,
		.read_nrt = edf_rtdm_read_nrt,
		.write_nrt = edf_rtdm_write_nrt,
	},
	.device_class = RTDM_CLASS_EXPERIMENTAL,
	.device_sub_class = SOME_SUB_CLASS,
	.profile_version = 1,
	.driver_name = "EDF-SCHED",
	.driver_version = RTDM_DRIVER_VER(0, 0, 1),
	.peripheral_name = "EDF Scheduler",
	.provider_name = "Luca Mella",
	.proc_name = device.device_name,
};
/*
* START/SWITCH Task callback 
*/
void edf_startswitch_hook( void * cookie )
{
	int status;
	status=rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
	if (status){
		rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
		return;
	}
	rt_dtask_recalculateprio();
	status=rt_mutex_release(&edf_data_mutex);
	if (status){
		rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
		return;
	}
}
/*
* DELETE Task callback 
* 
*/
void edf_delete_hook( void * cookie )
{
	int status;
	RT_TASK * task;
	task = T_DESC(cookie);
	
	status=rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
	if (status){
		rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
		return;
	}
	
	rt_dtask_delete(task);
	rt_dtask_recalculateprio();
	status=rt_mutex_release(&edf_data_mutex);
	
	if (status){
		rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
		return;
	}
}
/**
* This function is called when the module is loaded.
* It simply registers the RTDM device and initialize data structures.
*/
int __init edf_rtdm_init(void)
{
	int res;
	res = rtdm_dev_register(&device);
	if(res == 0) {
		rtdm_printk("EDF sched driver registered without errors\n");
		rt_dtask_init();
		rt_mutex_create( &edf_data_mutex , DMUTEX_NAME );
		rt_task_add_hook(T_HOOK_START|T_HOOK_SWITCH, &edf_startswitch_hook);
		rt_task_add_hook(T_HOOK_DELETE, &edf_delete_hook);
	}
	else {
		rtdm_printk("EDF sched driver registration failed: \n");
	    	switch(res) {
	      case -EINVAL:
	      	rtdm_printk("The device structure contains invalid entries. Check kernel log for further details.");
	      break;
	      case -ENOMEM:
	      	rtdm_printk("The context for an exclusive device cannot be allocated.");
	      break;
	      case -EEXIST:
	      	rtdm_printk("The specified device name of protocol ID is already in use.");
	      break;
	      case -EAGAIN: 
	      	rtdm_printk("Some /proc entry cannot be created.");
	      break;
	      default:
	      	rtdm_printk("Unknown error code returned");
	      break;
	      }
	  	rtdm_printk("\n");
	  }
	return res;
}

/**
* This function is called when the module is unloaded.
* It unregister the RTDM device and free data structures.
*
*/
void __exit edf_rtdm_exit(void)
{
	rtdm_printk("EDF-SCHED: stopping\n");
	rt_task_remove_hook(T_HOOK_START|T_HOOK_SWITCH, &edf_startswitch_hook);
	rt_task_remove_hook(T_HOOK_DELETE, &edf_delete_hook);  
	rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
	rt_dtask_dispose();
	rt_mutex_release(&edf_data_mutex);
	rt_mutex_delete(&edf_data_mutex);
	rtdm_dev_unregister(&device, 1000);
	rtdm_printk("EDF-SCHED: uninitialized\n");
}

module_init(edf_rtdm_init);
module_exit(edf_rtdm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luca Mella <lucam.ko@gmail.com>");
MODULE_DESCRIPTION("EDF Scheduler support");
MODULE_VERSION("0.1a");

