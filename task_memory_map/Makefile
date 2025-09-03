# Specify the kernel module(s) to build
obj-m += task_mm_struct.o    # Add 'task_mm_struct.o' as a module to be compiled

# Default target to build the module
all:
	# Build the module using the kernel build system
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target to remove generated files
clean:
	# Clean the module build files using the kernel build system
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
