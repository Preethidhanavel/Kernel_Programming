# Build object file (misc_driver.c → misc_driver.o → misc_driver.ko)
obj-m += misc_driver.o  

# "make" will build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "make clean" will remove generated files
clean:
	# clean build files from current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
