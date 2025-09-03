# Tell the build system we want to build log_level.c as a kernel module
obj-m += log_level.o 

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove generated files (like .o, .ko, .mod.c etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
