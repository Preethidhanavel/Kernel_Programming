# Specify the kernel module object to build
obj-m += ioctl.o        # Build ioctl.c as a kernel module

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove compiled module files
clean:
	# Clean all module build files in the current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
