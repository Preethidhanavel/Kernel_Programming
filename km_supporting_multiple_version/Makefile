# Tell the kernel build system to build multiple_version.o as a module
# The output will be multiple_version.ko
obj-m += multiple_version.o

# 'all' target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
