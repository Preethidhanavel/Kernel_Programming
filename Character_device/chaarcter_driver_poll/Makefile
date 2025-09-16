# Tell the kernel build system to build poll_driver.c into poll_driver.ko
obj-m += poll_driver.o

# "all" target: builds the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes temporary files and the built .ko module
clean:
	# Clean module build files from the current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
