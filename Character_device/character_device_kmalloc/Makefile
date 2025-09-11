# Specify the object file (kernel module) to build
obj-m += char_device_kmalloc.o

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove compiled files
clean:
	# Clean module build files in current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
