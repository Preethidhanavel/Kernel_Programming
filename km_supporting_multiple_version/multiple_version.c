#include <linux/kernel.h>   // Needed for printk()
#include <linux/module.h>   // Needed for all modules
#include <linux/version.h>  // Needed for kernel version macros

MODULE_LICENSE("GPL");      // Specify license type 

// Initialization function
static int my_init(void)
{
    // Check the running kernel version using preprocessor conditionals
    #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,0,0)
        printk(KERN_INFO "kernel version: old one \n");
    #elif LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
        printk(KERN_INFO "kernel version: new one \n");
    #else
        printk(KERN_INFO "kernel version: moderate one\n");
    #endif

    // Print the numeric value of the current kernel version
    printk("%u\n", LINUX_VERSION_CODE);

    return 0;
}

// Exit function
static void my_exit(void)
{
    printk(KERN_INFO "module removed\n");
}

// Register initialization and cleanup functions
module_init(my_init);
module_exit(my_exit);
