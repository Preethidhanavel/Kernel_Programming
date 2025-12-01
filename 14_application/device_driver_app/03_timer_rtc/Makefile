# This tells the kernel build system to build file_init_exit.c as a module (.ko file)
obj-m += file_init_exit.o

# "all" target - used to compile the kernel module
all:
	# Calls kernel build system and tells it to compile the module in current directory (M=$(PWD))
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target - removes all build-generated temporary and compiled files
clean:
	# Calls kernel build system to remove generated object files, .ko, .o, .mod files etc.
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
