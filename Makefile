all:
	gcc ./SingleFileFS/SingleFileSystem_Create.c -o SingleFileFS/a.out
	make -f SystemCallFind/Makefile compile-extern
	sudo make -f SystemCallFind/Makefile insmod-extern
	make -f SingleFileFS/Makefile compile-extern
	sudo make -f SingleFileFS/Makefile insmod-extern
clean:
	make -f SingleFileFS/Makefile clean-extern
	make -f SingleCallFind/Makefile clean-extern

insmod:
	sudo make -f SingleFileFS/Makefile insmod-extern

rmmod:
	rm SingleFileFS/a.out
	sudo make -f SingleFileFS/Makefile rmmod-extern
	sudo make -f SystemCallFind/Makefile rmmod-extern

create-fs:
	dd bs=4096 count=100 if=/dev/zero of=the-device
	./SingleFileFS/a.out the-device
	rm -df PointMount
	mkdir PointMount
mount-fs:
	mount -o loop -t MyFileSystem the-device ./PointMount
create-user:
	gcc user/main.c -o user/a.out
