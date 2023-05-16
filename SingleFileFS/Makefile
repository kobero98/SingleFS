obj-m := SingleFileSystem.o
SingleFileSystem-objs += file.o dir.o SingleFS.o
KDIR := /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS:= -D NBLOCK=10 SYSTEMCALLTABLE=$(shell cat /sys/module/the_usctm/parameters/sys_call_table_address) ARRAY_FREE_ENTRY=$(shell cat /sys/module/the_usctm/parameters/free_entries) 

all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean 
insmod:
	insmod SingleFileSystem.ko
rmmod:
	rmmod SingleFileSystem.ko
compile-extern:
	make -C $(KDIR) M=$(PWD)/SingleFileFS modules
clean-extern:
	make -C $(KDIR) M=$(PWD)/SingleFileFS clean 
