# Specify the kernel module to build
obj-m += tasklet_static.o    

# Build target: compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove all generated files from module build
clean:
	# Clean the module build files
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
