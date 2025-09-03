# Tell the kernel build system to build kthread_run.o as a module
# This will generate kthread_run.ko after compilation
obj-m += kthread_run.o

# 'all' target: used to compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated build files
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
