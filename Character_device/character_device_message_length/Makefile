# Build target for our kernel module
obj-m += character_device_count.o

# 'all' target: used to build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: used to remove built files
clean:
	# Call kernel build system with 'clean' to remove temporary build files
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
