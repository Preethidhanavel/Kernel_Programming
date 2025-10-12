# Build prDebug.o as a loadable kernel module
obj-m += prDebug.o

# Optional: define DEBUG macro for conditional debugging in module source
# ccflags-y := -DDEBUG

# 'all' target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
