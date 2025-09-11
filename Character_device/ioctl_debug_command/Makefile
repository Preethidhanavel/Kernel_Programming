# Specify the kernel module object to build
obj-m += ioctl_cmd.o        # Compile ioctl_cmd.c as a kernel module

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove compiled module files
clean:
	# Cleans all temporary and compiled module files in the current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
