obj-m += the_usctm.o
the_usctm-objs += usctm.o ./lib/vtpmo.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
insmod:
	sudo insmod the_usctm.ko
rmmod:
	sudo rmmod the_usctm.ko 
insmod-extern:
	sudo insmod SystemCallFind/the_usctm.ko
rmmod-extern:
	sudo rmmod SystemCallFind/the_usctm.ko 
compile-extern:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/SystemCallFind modules 
clean-extern:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD)/SystemCallFind clean