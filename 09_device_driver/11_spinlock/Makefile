# Specify the kernel module to build
obj-m += spinlock.o

all:
	# Build the module using the kernel build system
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	# Clean all generated files in the current module directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
