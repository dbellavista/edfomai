obj-m += edfomai-drv.o
edfomai-drv-objs := edfomai-drv-data.o

EXTRA_CFLAGS += -I/usr/include/xenomai/

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
