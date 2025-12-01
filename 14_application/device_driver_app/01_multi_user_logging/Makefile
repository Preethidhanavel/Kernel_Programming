# This line tells the kernel build system to build two modules:
# multi_user.o → becomes multi_user.ko
# k_to_k.o     → becomes k_to_k.ko
obj-m += multi_user.o k_to_k.o 



# - Calls kernel build system located in /lib/modules/$(uname -r)/build
# - M=$(PWD) means: build modules located in current directory
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# - Cleans temporary files generated while building modules
# - Removes *.o, *.ko, *.mod, .tmp etc.
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
