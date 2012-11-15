#include <linux/module.h>
#include <native/mutex.h>
#include "edfomai-data.h"
#include "edfomai-drv-data.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luca Mella <lucam.ko@gmail.com>");
MODULE_DESCRIPTION("EDF Scheduler support");

#define DEVICE_NAME "edf-scheduler"
#define SOME_SUB_CLASS 4711

/**
* The context of a device instance
*
* A context is created each time a device is opened and passed to
* other device handlers when they are called.
*
*/
typedef struct context_s
{
} context_t;

// Access ti edf data must be coordinated throug this mutex
static RT_MUTEX edf_data_mutex;

/**
* Open the device
*
* This function is called when the device shall be opened.
*
*/
static int edf_rtdm_open_nrt(struct rtdm_dev_context *context, rtdm_user_info_t *user_info, int oflags)
{
  //context_t *ctx = (context_t*)context->dev_private;. 
  return 0;
}

/**
* Close the device
*
* This function is called when the device shall be closed.
*
*/
static int edf_rtdm_close_nrt(struct rtdm_dev_context *context, rtdm_user_info_t * user_info)
{
  return 0;
}

/**
* Read from the device
*
* This function is called when the device is read in non-realtime
* context.
*
*/
static ssize_t edf_rtdm_read_nrt(struct rtdm_dev_context *context, rtdm_user_info_t * user_info, void *buf, size_t nbyte)
{
  // TODO: copy to user some kind of useful information for monitoring purposes..
  return -1;
}

/**
* Write in the device
*
* This function is called when the device is written in non-realtime context.
*
_*/
static ssize_t edf_rtdm_write_nrt(struct rtdm_dev_context *context, rtdm_user_info_t * user_info, const void *buf, size_t nbyte)
{
  //context_t *ctx = (context_t*)context->dev_private;
  struct EDFMessage message;
  int res,status;

  if (nbyte != sizeof(struct EDFMessage))
	return -1; // You should talk to EDF-SCHED with EDFMessages
 
  if ( rtdm_copy_from_user( user_info, &message, buf, sizeof(EDFMessage) ) == EFAULT )
  	return EFAULT;
 
  switch (message.command)
  {
        case CREATE_TASK:
                status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
                if (status)
                {
                        rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
                        return status;
                }

                rt_dtask_create ( &(message.task) , message.deadline );
                // Task will be started outside the module because we need to pass an user function pointer to the new task

                status =rt_mutex_release(&edf_data_mutex);
                if (status)
                {
                        rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
                        return status;
                }

        break; 
	case START_TASK:
		status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
		if (status)
		{
			rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
			return status;
		}
		// TODO: add task info to data structures. 
		rt_dtask_setdeadline ( &(message.task) , message.deadline );
		// Task will be started outside the module because we need to pass an user function pointer to the new task

                status =rt_mutex_release(&edf_data_mutex);
                if (status)
                {
                        rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
                        return status;
                }

	break;		
	case RESET_DEADLINE:
		status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
		if (status)
		{
			rtdm_printk("%s acquire failed with status %d", DMUTEX_NAME, status);
			return status;
		}
		//TODO: re-set the deadline with previous value (not the specified one?) 
                rt_dtask_resetdeadline ( &(message.task) );

                status =rt_mutex_release(&edf_data_mutex);
                if (status)
                {
                        rtdm_printk("%s release failed with status %d", DMUTEX_NAME, status);
                        return status;
                }
	break;
  }
  return nbyte;
}



/**
* -------------------------------------------------------------
* This structure describe the simple RTDM device
*
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
* START/SWITCH TASK Callback 
* 
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
* DELETE  TASK Callback 
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
* This function is called when the module is loaded
*
* It simply registers the RTDM device.
*
*/
int __init edf_rtdm_init(void)
{
  int res;

  res = rtdm_dev_register(&device);
  if(res == 0) {
    rtdm_printk("EDF sched driver registered without errors\n");
    rt_dtask_init();
    rt_mutex_create( &edf_data_mutex , DMUTEX_NAME );
    rt_task_addhook(T_HOOK_START|T_HOOK_SWITCH, &edf_startswitch_hook);
    rt_task_addhook(T_HOOK_DELETE, &edf_delete_hook);
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
* This function is called when the module is unloaded
*
* It unregister the RTDM device, polling at 1000 ms for pending users.
*
*/
void __exit pwm_rtdm_exit(void)
{
  int status;
  rtdm_printk("EDF-SCHED: stopping\n");
  // TODO: cleanup
  rt_task_removehook(T_HOOK_START|T_HOOK_SWITCH, &edf_startswitch_hook);
  rt_task_removehook(T_HOOK_DELETE, &edf_delete_hook);
  
  status =rt_mutex_acquire(&edf_data_mutex, TM_INFINITE);
  
  rt_dtask_dispose();
  status =rt_mutex_release(&edf_data_mutex);
  
  rt_mutex_delete(&edf_data_mutex);

  rtdm_dev_unregister(&device, 1000);
  rtdm_printk("EDF-SCHED: uninitialized\n");
}


module_init(edf_rtdm_init);
module_exit(edf_rtdm_exit);

