# Build preprocess.o as a loadable kernel module
obj-m += preprocess.o

# Define DEBUG macro only for preprocess.o compilation
CFLAGS_preprocess.o := -DDEBUG

# Alternative way to define DEBUG for all modules in directory
# ccflags-y := -DDEBUG

# 'all' target: compile the module
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

# 'clean' target: remove all generated build files (.o, .ko, .mod.c, etc.)
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
