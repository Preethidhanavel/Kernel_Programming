# Add kernel module object file
obj-m += signal.o

# Build all modules
all:
	# Use kernel build system to compile module in current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean generated files
clean:
	# Use kernel build system to clean module build artifacts
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
