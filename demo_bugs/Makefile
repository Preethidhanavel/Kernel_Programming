# This tells the kernel build system to build a module named demo_bug.ko
# (from demo_bug.c → demo_bug.o → demo_bug.ko)
obj-m += demo_bug.o

# "all" target: used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes all generated build files
# This clears temporary files like *.o, *.ko, *.mod.c, .symvers, .order, etc.
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
