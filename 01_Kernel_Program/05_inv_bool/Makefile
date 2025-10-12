# Tell the kernel build system to build 'inv_bool.o' as a loadable module (.ko)
obj-m += inv_bool.o

# 'all' target: used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: used to clean up all temporary build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
