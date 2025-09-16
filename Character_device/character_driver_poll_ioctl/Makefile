# Build object file poll_ioctl.o into a loadable kernel module poll_ioctl.ko
obj-m += poll_ioctl.o

# "all" target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated build files
clean:
	# Clean up module build artifacts (.o, .ko, .mod.c, etc.)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
