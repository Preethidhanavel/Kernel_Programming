# This tells the kernel build system to build a module named current.ko
# (from current.c → current.o → current.ko)
obj-m += current.o

# "all" target: used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated files (like .o, .ko, .mod.c, .order, .symvers)
# Keeps your directory clean for a fresh build
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
