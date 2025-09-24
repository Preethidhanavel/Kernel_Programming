# Specify the kernel module to build
obj-m += tasklet_dynamic.o    

# Build target: compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove all generated module build files
clean:
	# Clean the module build files
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
