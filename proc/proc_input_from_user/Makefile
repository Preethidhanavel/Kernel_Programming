# Tell the kernel build system to build proc_input_user.c
# into a loadable kernel module (proc_input_user.ko)
obj-m += proc_input_user.o

# "all" target: build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated files (.o, .ko, .mod, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
