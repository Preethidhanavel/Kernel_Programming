# Build a kernel module named KM_no_initExit.ko
obj-m += KM_no_initExit.o

# 'all' target: used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove temporary build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
