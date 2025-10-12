# Build two separate kernel modules: hello.ko and hello1.ko
# Each source file (hello.c, hello1.c) will be compiled into its own module
obj-m += hello.o
obj-m += hello1.o

# 'all' target: compile the kernel modules
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all temporary build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
