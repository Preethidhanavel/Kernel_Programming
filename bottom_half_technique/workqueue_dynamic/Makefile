# Specify the kernel module to be built (workqueue_dynamic.ko)
obj-m += workqueue_dynamic.o    

# Build target: compiles the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: removes all generated files
clean:
	# Clean the build files for this module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
