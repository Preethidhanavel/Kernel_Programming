# Build a kernel module named world.ko
obj-m += world.o

# 'all' target: compiles the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: removes all temporary build files
# (.o, .ko, .mod.c, Module.symvers, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
