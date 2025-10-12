# Tell the kernel build system to build no_license.c into a loadable module (no_license.ko)
obj-m += no_license.o 

# "all" target: compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
