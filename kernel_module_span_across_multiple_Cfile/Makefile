# Build a kernel module named 'linux.ko'
obj-m += linux.o

# This means hello.c and func.c will be compiled and then linked into linux.ko
linux-objs := hello.o func.o

# 'all' target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove generated files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
