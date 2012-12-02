Edfomai
=======
Edf implememtation over Xenomai
ALMA MATER STUDIORUM - University of Bologna
Computer Engineering, Sistemi e controllo per l'automazione

DevTeam: Bellavista Daniele,Mella Luca,Ricci Luca

#### Architecture
-------
Other approaches implement EDF scheduling policy directly in inside the kernel scheduler,
extending the Xenomai Nucleus and adding new user API (eg.Native skin).
Our approach is slighty different in fact we do not modify any line of code of the Xenomai patch, we exploit Xenomai
low level APIs to build a Kernel module that hooks to the Kernel scheduler and changes tasks priorities dinamically.
In order to avoid new syscall definition (and consequential Kernel modification) we decide to develop a device-driver
module and by wrapping user/driver communication we can also provide a handy user API, pretty similar to the native one.

* `.----------.                            `
* `| RT Task  |-------------------.        `
* `'----|-----'                   |        `
* `     |                         |        `
* `.----|--------------.    ,-----|-------.`
* `| Edfomai User API  | __ | Native skin |`  
* `'---|---------------'    '-------------'`
* `    |                                   `   
* `.---|-----,----------------------------.`  
* `| Edfomai |       Kernel               |`
* `| Driver  |      (Nucleus)             |`   
* `'---------'----------------------------'`

Edfomai Driver module doesn't simply react to user messages, it also starts a priority recalculation service which is wake up every time the scheduler is invoked, in order to safely recalculate and set current tasks priority.

#### Techincal Notes
-------
Here we summarize the key-point of our EDF scheduling policy implementation, in particular we'll show most interesting 
Xenomai APIs that had enabled us to build the system.

* `rt_task_add_hook` : permit to add an user defined hook procedure that will be invoked whenever scheduler is invoked. This API is reachable in kernel space only (so we develop a kernel module)
* `rt_task_set_priority` : set a new priority value to a given task. This function might casuse a rescheduling, in other worlds it can invoke the scheduling procedure and consequentially trigger registered scheduler's hook. (This might lead to a system hang)
* `xnregistry_bind` : which permit to obtain a `xnhandle_t` (which is a pointer to a `xnobject_t`) by specifying a key string (in our case the task name). This function in part on the Nucleus API, so its reachable only in kernel space.
* `xnregistry_fetch` : obtain the corresponting real time object of a given `xnhandle_t`. In our case its used to fetch a thread object so it returns a pointer to a `xnthread_t`.
* `T_DESC` : macro that retrieve a pointer to the `RT_TASK` structure wich contains the specified `xnthread_t` as thread_base

After this quick glance to these APIs it's pretty immediate understand the reason which made us move to the architecure descripted above, in fact priority recalculation service avoid kernel hanging caused by the recoursive invocation of scheduler, hook procedure and set_priority. 

Deadline related data has been stored in ad hoc data structures where relative deadline, absolute deadline and remaining time are stored. For the sake of simplicity (lack of time) we decided to adapt `uthash` hash map to kernel space, this way we obtain a scheduling complexity of `O(2N)` which should not be the maximum performance reachable, but it's acceptable for our purposes.

#### Build
-------
To correctly build the module patched kernel header are required (we haven't tested it in case of unpatched kernel header) and Xenomai framework should be installed (we compile demo code using xenomai generated gcc flags).
Once these requirements are satisfied:
* `make` : builds driver module `edfomai.ko` and demos (both sporadic and periodic)
* `make dirver` : builds only driver module
* `make demo` : builds only demos
* `make install` : install `edfomai.ko` as system module (then it could be loaded and unloaded with `modprobe`)
* `make clean` : clean both driver and demo
Then module could be mounted with `insmod edfomai.ko` (or `modprobe edfomai`) and unmounted with `rmmod edfomai`. Debugging print are still enabled in Makefile configuration, so `dmesg|grep Edfomai` output could be sufficiently verbose for better understanding of the events.

#### Release Note
-------
At the moment this project is not meant to be a professional edf scheduler, further improvements and test should be done in order to achive that maturity. So, currently, this project remains an academic project (just a draft).
Hope someone help us in improving it :)

#### License
-------
GPL v2
