# Tell the kernel build system to build 'array_parameter.o' as a module
obj-m += array_parameter.o

# 'all' target: this is used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: removes all temporary build files for a fresh build
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
