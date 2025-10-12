# List of object files to be built as kernel modules
# Here,building two modules: export_function.o and add_fun.o
obj-m += export_function.o add_fun.o

# Command to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Command to clean the build files
clean:
	# This will clean all temporary files generated during module compilation
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
