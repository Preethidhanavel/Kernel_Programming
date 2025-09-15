# Build target: name of your kernel module object
obj-m += non_block_waitQueue.o  

# 'make' rule to build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# 'make clean' rule to remove generated build files
clean:
	# Ask the kernel build system to clean up the module build files
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  
