# Build object file from completion.c
obj-m += completion.o    

# Default target to build the kernel module
all:
	# Use the kernel build system to compile this directory's module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Clean target to remove all generated files
clean:
	# Ask kernel build system to clean module build artifacts
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  
