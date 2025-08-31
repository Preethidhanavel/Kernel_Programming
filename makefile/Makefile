# Build a kernel module named 'linux.ko'
# 'linux.o' is the main target module
obj-m += linux.o

# That means my_module.c -> my_module.o -> linked into linux.ko
linux-objs := my_module.o

# 'all' target: compile the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all temporary build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
