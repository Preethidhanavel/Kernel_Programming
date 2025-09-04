# Tell the build system to build parameter_pass.c into parameter_pass.ko
obj-m += parameter_pass.o   

# Default target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# Clean target: remove generated files (like .o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
