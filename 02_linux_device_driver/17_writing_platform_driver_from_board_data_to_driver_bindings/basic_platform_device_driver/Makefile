# List of object files to be built as modules
# device1.c → device module
# driver1.c → driver module
obj-m += device1.o driver1.o

# 'all' target: build the kernel modules
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated files
clean:
	# Ask kernel build system to clean up current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
