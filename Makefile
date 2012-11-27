ifneq ($(KERNELRELEASE),)
# We were called by kbuild

obj-m := edfomai-drv.o
edfomai-drv-y := edfomai-drv-data.o # edfomai-data.o

ccflags-y := -I/lib/modules/$(shell uname -r)/build/include/xenomai

else  # We were called from command line

KDIR := /lib/modules/$(shell uname -r)/build
#KDIR := /home/cynove/src/kernel/linux-source-2.6.31
PWD  := $(shell pwd)

#EXTRA_CFLAGS += -I/lib/modules/$(shell uname -r)/build/include/xenomai  # -I/usr/include/xenomai/

default:
	@echo '    Building target module 2.6 kernel.'
	@echo '    PLEASE IGNORE THE "Overriding SUBDIRS" WARNING'
	#$(CC) -C edfomai-data.c $(EXTRA_CFLAGS)
	#$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	$(MAKE) -C $(KDIR) M=$(PWD)
	

install:
	$(MAKE) INSTALL_MOD_DIR=edfomai -C $(KDIR) M=$(PWD) modules_install
	#./do_install.sh *.ko

endif  # End kbuild check
######################### Version independent targets ##########################

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f -r *.o *.ko .*cmd .tmp* core *.i

