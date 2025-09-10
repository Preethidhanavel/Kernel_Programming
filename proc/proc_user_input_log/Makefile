# Build target: compile proc_input_user_log.c into a kernel module
# This will produce proc_input_user_log.ko
obj-m += proc_input_user_log.o

# "all" target: default when you run `make`
# It calls the kernel build system to build our module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all temporary build files
# This keeps only the source code and Makefile
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
