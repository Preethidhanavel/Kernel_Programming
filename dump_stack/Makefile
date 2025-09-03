# This tells the kernel build system to build a module named stack_dump.ko
# (from stack_dump.c → stack_dump.o → stack_dump.ko)
obj-m += stack_dump.o

# "all" target: compiles the kernel module
# modules : keyword to trigger external module compilation
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: removes all generated files from the build
# Deletes temporary files like *.o, *.ko, *.mod.c, .symvers, .order, etc.
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
