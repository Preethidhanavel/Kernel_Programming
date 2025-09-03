#include <linux/kernel.h>      // Needed for printk and kernel macros
#include <linux/module.h>      // Needed for all kernel modules
#include <generated/utsrelease.h> // Provides UTS_RELEASE macro 

MODULE_LICENSE("GPL");         // Declare GPL license

// Module initialization function
static int my_init(void)
{
    // Print the kernel version using UTS_RELEASE
    printk(KERN_INFO "init function\t kernel_version: %s\n", UTS_RELEASE);
    return 0;  // Return 0 indicates successful initialization
}

// Module cleanup function
static void my_exit(void)
{
    // Inform that module is being removed
    printk("exit function\n");
}

// Register init and exit functions
module_init(my_init);
module_exit(my_exit);
