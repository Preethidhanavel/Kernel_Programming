# Name of the object file to be built as a module
obj-m += hex_dump.o

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove all generated files
clean:
	# Call kernel build system to clean module build artifacts
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
