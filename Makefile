all:
	gcc ./SingleFileFS/SingleFileSystem_Create.c -o SingleFileFS/a.out
	make -f SystemCallFind/Makefile compile-extern
	sudo make -f SystemCallFind/Makefile insmod-extern
	make -f SingleFileFS/Makefile compile-extern
	sudo make -f SingleFileFS/Makefile insmod-extern
clean:
	make -f SingleFileFS/Makefile clean-extern
	make -f SingleCallFind/Makefile clean-extern
	rm a.out
insmod:
	sudo make -f SingleFileFS/Makefile insmod-extern

rmmod:
	sudo make -f SingleFileFS/Makefile rmmod-extern
	sudo make -f SystemCallFind/Makefile rmmod-extern

create-fs:
	dd bs=4096 count=100 if=/dev/zero of=the-device
	gcc -D NBLOCK=100 ./SingleFileFS/SingleFileSystem_Create.c -o ./SingleFileFS/a.out
	./SingleFileFS/a.out the-device
	rm -df PointMount
	mkdir PointMount
mount-fs:
	sudo mount -o loop -t MyFileSystem the-device ./PointMount
umount-fs:
	sudo umount PointMount
create-user:
	gcc user/main.c -o user/a.out

