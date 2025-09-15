# Specify the kernel object to build
obj-m += charDev_track_count.o   # The module source file to compile

# Default target: build the module
all:
	# Invoke the kernel build system to compile modules
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove compiled files
clean:
	# Invoke the kernel build system to clean module build artifacts
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
