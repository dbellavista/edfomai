obj-m += edfomai.o
edfomai-objs := edfomai-drv.o edfomai-drv-data.o

#$(shell pwd)/edfomai-drv-data.o: $(shell pwd)/edfomai-drv-data.h

EXTRA_CFLAGS += -I/usr/include/xenomai/
EXTRA_CFLAGS += -DDEBUG 

all: driver demo

driver:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

demo: 
	make -f Makefile.demo

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	make -f Makefile.demo clean
	
