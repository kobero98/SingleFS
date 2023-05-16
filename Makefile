all:
	gcc ./SingleFileFS/SingleFileSystem_Create.c
	make -f SingleFileFS/Makefile compile-extern
clean:
	rm a.out
	make -f SingleFileFS/Makefile clean-extern
insmod:
	insmod SingleFileFS/SingleFileSystem.ko
rmmod:
	rmmod SingleFileFS/SingleFileSystem.ko
create-fs:
	dd bs=4096 count=100 if=/dev/zero of=the-device
	./SingleFileFS/a.out the-device
	rm -df PointMount
	mkdir PointMount
mount-fs:
	mount -o loop -t MyFileSystem the-device ./PointMount