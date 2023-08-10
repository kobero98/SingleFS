obj-m := SingleFileSystem.o
SingleFileSystem-objs += file.o dir.o SingleFS.o systemcallFS.o
KDIR := /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS:= -D NBLOCK=10
systemcalltable := $(shell cat /sys/module/the_usctm/parameters/sys_call_table_address)
array_free_entries := $(shell cat /sys/module/the_usctm/parameters/free_entries) 
all:
	make -C $(KDIR) M=$(PWD) modules
clean:
	make -C $(KDIR) M=$(PWD) clean 
insmod:
	insmod SingleFileSystem.ko systemcall_table=$(systemcalltable) free_entries=$(array_free_entries)
insmod-extern:
	insmod SingleFileFS/SingleFileSystem.ko systemcall_table=$(systemcalltable) free_entries=$(array_free_entries)
rmmod-extern:
	rmmod -f SingleFileFS/SingleFileSystem.ko
rmmod:
	rmmod -f SingleFileSystem.ko
compile-extern:
	make -C $(KDIR) M=$(PWD)/SingleFileFS modules
clean-extern:
	make -C $(KDIR) M=$(PWD)/SingleFileFS clean 
