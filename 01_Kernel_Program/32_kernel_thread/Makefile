# This line tells the kernel build system to build kthread.o as a module
# The result will be a loadable kernel module file: kthread.ko
obj-m += kthread.o

# 'all' target: Compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: Remove all generated files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
