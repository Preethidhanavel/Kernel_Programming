# Tell the kernel build system that we want to build export_function.c
# into a loadable kernel module named export_function.ko
obj-m += export_function.o

# 'all' target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove the temporary files generated during build
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
