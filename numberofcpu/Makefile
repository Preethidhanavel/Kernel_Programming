# Build find_cpu.o as a loadable kernel module
# Output will be find_cpu.ko
obj-m += find_cpu.o

# 'all' target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
