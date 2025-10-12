# Build target object (kernel module name = linked_list.ko)
obj-m += linked_list.o    

# Default target: build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove generated build files
clean:
	# Clean the module build files from current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
