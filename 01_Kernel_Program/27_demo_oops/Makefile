# This tells the kernel build system to build a module named demo_oops.ko
# (from demo_oops.c → demo_oops.o → demo_oops.ko)
obj-m += demo_oops.o

# "all" target: used to compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes all generated build files
# Deletes temporary files like *.o, *.ko, *.mod.c, .symvers, .order, etc.
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
