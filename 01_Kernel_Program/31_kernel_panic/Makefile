# Specify the object file to be built as a kernel module
# This will create demo_panic.ko
obj-m += demo_panic.o

# 'all' target: Build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: Remove all generated files (object files, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
