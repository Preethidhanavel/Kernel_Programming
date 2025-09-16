# Build two kernel modules: charDev_modular.ko and export_driver1.ko
obj-m += charDev_modular.o export_driver1.o

# "all" target: compile the modules
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# "clean" target: remove build files (.o, .ko, .mod.c, etc.)
clean:
	# Clean module build files from current directory
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
