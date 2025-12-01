obj-m += driver.o helper.o


# "make all" or simply "make" command triggers 
# kernel build system to compile the modules.

all:
	make -C /lib/modules/`uname -r`/build M=$(PWD) modules



# Clean rule to remove generated .ko, .o, and temp build files.
# Helps keep directory clean for fresh builds.

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
