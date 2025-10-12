# List of object files to build as kernel modules
obj-m += char_device_udev.o   # This tells the kernel build system to build char_device_udev.c

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove generated files
clean:
	# Same as above but runs the 'clean' target in kernel build system
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
