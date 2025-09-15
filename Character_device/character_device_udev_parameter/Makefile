# Target module name
obj-m += charDev_udev_parameter.o

# Build the module using the kernel build system
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean up generated files
clean:
	# Removes temporary files created during build (like .o, .ko, .mod.c, etc.)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
