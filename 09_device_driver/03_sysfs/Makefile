# Specify the kernel module to be built
obj-m += sysfs_drv.o    

# Default target: build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove generated files
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
