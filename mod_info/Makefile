# Tell the kernel build system to build mod_info.o as a module
obj-m += mod_info.o    

# Rule to build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules  

# Rule to clean build files
clean:
	# Clean up the generated files in current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean  
