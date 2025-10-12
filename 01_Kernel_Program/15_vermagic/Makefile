# Tell the kernel build system to build vermagic.c into a loadable module (vermagic.ko)
obj-m += vermagic.o 

# "all" target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
