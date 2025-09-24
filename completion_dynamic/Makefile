# Build object file (completion_dynamic.c → completion_dynamic.o → completion_dynamic.ko)
obj-m += completion_dynamic.o  

# "make" will build the kernel module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "make clean" will remove generated files
clean:
	# clean build files from current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
