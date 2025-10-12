# This tells the kernel build system to build a module named current_macro.ko
# (from current_macro.c → current_macro.o → current_macro.ko)
obj-m += current_macro.o

# "all" target: used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes all generated build files
# This deletes files like *.o, *.ko, *.mod.c, .symvers, .order
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
