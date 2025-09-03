# Tell the kernel build system which object file to build as a module
# This will create initdata_macro.ko after compilation
obj-m += initdata_macro.o

# 'all' target: builds the module using the current kernel's build system
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: cleans the build artifacts (object files, .ko, .o, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
