# Specify the kernel module object file to build
obj-m += kernel_timer.o    

# Default target to build the kernel module
all:
	# Use the kernel build system to compile the module in this directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Clean target to remove all generated files
clean:
	# Ask the kernel build system to clean build artifacts for this module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  
