# Tell the kernel build system which module to build
# Here, we are building char_device_mutex_protect.c into a .ko module
obj-m += char_device_mutex_protect.o  

# The 'all' target: this calls the kernel build system to build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# The 'clean' target: removes all temporary build files
clean:
	# Cleans up the module build files created in the current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  
