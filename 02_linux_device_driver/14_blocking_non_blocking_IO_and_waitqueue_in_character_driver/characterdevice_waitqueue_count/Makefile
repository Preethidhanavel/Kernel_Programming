# Specify the kernel module object to be built
obj-m += waitQueue_reader_count.o  

# Rule to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Rule to clean the build output
clean:
	# Ask the kernel build system to remove all generated files for this module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  
