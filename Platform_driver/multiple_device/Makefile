# List of kernel modules to build
# device_2.o → platform device module
# driver_2.o → platform driver module
obj-m += device_2.o driver_2.o

# 'all' target: build the modules
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove build-generated files
clean:
	# Ask kernel build system to clean all module-related files
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
