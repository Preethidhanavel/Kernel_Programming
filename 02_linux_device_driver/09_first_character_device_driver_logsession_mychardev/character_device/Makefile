# Tell the kernel build system which object file to build as a module
obj-m += character_device.o    

# "make" will build the module using the kernel build system
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "make clean" will clean up all generated files (like .o, .ko, .mod.c etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
