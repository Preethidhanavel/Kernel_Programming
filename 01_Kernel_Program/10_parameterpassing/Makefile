# Build a kernel module named parameter_pass.ko
obj-m += parameter_pass.o

# 'all' target: used to compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: removes all the temporary build files (like .o, .ko, .mod.c, Module.symvers, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
