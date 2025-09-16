# List of kernel modules to build
# device1.o → platform device module
# driver1.o → platform driver module
obj-m += device1.o driver1.o

# 'all' target: build the modules
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove build-generated files
clean:
	# Ask kernel build system to clean all module-related files
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
