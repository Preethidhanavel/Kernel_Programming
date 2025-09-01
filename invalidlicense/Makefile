# Name of the module to build (invalid_license.c -> invalid_license.ko)
obj-m += invalid_license.o  

# Rule to build the kernel module
all:
	# Use the kernel build system, passing current directory (M=$(PWD))
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Rule to clean the build files
clean:
	# Clean the module build files from the current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
