# Build target: kernel will compile poll_driver1.c into poll_driver1.ko
obj-m += poll_driver1.o

# "all" target: used to build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes all temporary build files (.o, .ko, .mod.c, etc.)
clean:
	# Clean build artifacts for this module
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
