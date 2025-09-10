# Build target: tell the kernel build system to build proc_file_uptime.c
# into a loadable kernel module named proc_file_uptime.ko
obj-m += proc_file_uptime.o

# "all" target: compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated build files (.o, .ko, .mod, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
