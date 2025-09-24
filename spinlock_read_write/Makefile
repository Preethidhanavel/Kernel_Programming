# Specify the module to be built
obj-m += spinlock_rw.o

# Build the module using the kernel build system
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean generated files
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
