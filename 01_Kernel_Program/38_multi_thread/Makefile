# Build kthread2.o as a kernel module
# The result will be kthread2.ko
obj-m += kthread2.o

# 'all' target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
