# Tell the kernel build system which object files to build as modules
# Here we are building two modules: modular_driver.ko and export_driver.ko
obj-m += modular_driver.o export_driver.o

# Default target: build the modules
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove generated files
clean:
	# Clean all module-related files from current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
