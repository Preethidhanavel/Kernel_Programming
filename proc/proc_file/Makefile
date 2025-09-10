# This line tells the kernel build system that we want to build proc_file.c
# into a loadable kernel module (proc_file.ko).
obj-m += proc_file.o

# "all" target: build the module using the kernel build system
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated files (like .o, .ko, .mod, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
