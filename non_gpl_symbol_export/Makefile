# Build target: two object files will be built into kernel modules
obj-m += export_function.o add_fun.o  

# Rule to build the modules
all:
	# Call the kernel build system in the current directory (M=$(PWD))
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Rule to clean the build files
clean:
	# Clean all generated files for this module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
