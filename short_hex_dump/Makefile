# Specify the kernel module(s) to build
obj-m += print_hex.o        # Add 'print_hex.o' as a module to be compiled

# Add extra compilation flags for this module
ccflags-y := -DDEBUG        # Define DEBUG macro when compiling print_hex.o

# Default target to build the module
all:
	# Build the module using the kernel build system
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target to remove generated files
clean:
	# Clean the module build files using the kernel build system
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
