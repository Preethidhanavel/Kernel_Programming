#include <linux/kernel.h>   // Needed for printk()
#include <linux/module.h>   // Needed for all modules
#include <linux/version.h>  // Needed for kernel version macros

MODULE_LICENSE("GPL");      // Specify license type 

// Initialization function
static int my_init(void)
{
    // Print the current kernel version code
    // LINUX_VERSION_CODE expands to an integer value representing the current kernel version
    printk(KERN_INFO "init function\t kernel_version:%u\n", LINUX_VERSION_CODE);

    // Print integer value corresponding to version 5.4.233
    printk(KERN_INFO "kernel_version 5.4.233 %u\n", KERNEL_VERSION(5,4,233));

    // Print integer value corresponding to version 4.5.3
    printk(KERN_INFO "kernel_version 4.5.3 %u\n", KERNEL_VERSION(4,5,3));

    return 0;
}

// Exit function
static void my_exit(void)
{
    printk("exit function\n");
}

// Register module initialization and cleanup functions
module_init(my_init);
module_exit(my_exit);
