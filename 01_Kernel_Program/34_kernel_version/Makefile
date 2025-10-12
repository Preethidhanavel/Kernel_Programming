# This line tells the kernel build system to build version.o as a module
# The result will be a kernel object file: version.ko
obj-m += version.o

# 'all' target: compiles the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: removes all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
