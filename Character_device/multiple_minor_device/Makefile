# Build target: our kernel module name (without .ko extension)
obj-m += charDev_multiple_minor.o  

# Command to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Command to clean build artifacts
clean:
	# Cleans up temporary build files created while compiling the module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
