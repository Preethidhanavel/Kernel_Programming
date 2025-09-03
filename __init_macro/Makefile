# Build three kernel modules: hello1.ko, hello2.ko, hello3.ko
obj-m += hello1.o hello2.o hello3.o

# "all" target: used to build the kernel modules

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes all the temporary build files
# This keeps the directory clean (like *.o, *.ko, *.mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
