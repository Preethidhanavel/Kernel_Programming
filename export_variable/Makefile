# Tell the build system to compile variable_export.c into a module (variable_export.ko)
obj-m += variable_export.o 

# "all" target: build the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: clean up generated files (like .o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
