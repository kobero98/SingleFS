obj-m := SingleFileSystem.o
SingleFileSystem-objs += file.o dir.o SingleFS.o
#SingleFileSystem-y := file.o dir.o
KDIR := /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS:= -D NBLOCK=3
all:
	gcc SingleFileSystem_Create.c 
	make -C $(KDIR) M=$(PWD) modules
clean:
	#rm a.out
	make -C $(KDIR) M=$(PWD) clean
insmod:
	insmod SingleFileSystem.ko
rmmod:
	rmmod SingleFileSystem.ko
create-fs:
	dd bs=4096 count=100 if=/dev/zero of=the-device
	./a.out the-device
	rm -d mount
	mkdir mount
create-fs2:
	dd bs=4096 count=100 if=/dev/zero of=the-device2
	./a.out the-device2
	rm -d mount2
	mkdir mount2
mount-fs:
	mount -o loop -t MyFileSystem the-device ./mount
mount-fs2:
	mount -o loop -t MyFileSystem the-device2 ./mount2