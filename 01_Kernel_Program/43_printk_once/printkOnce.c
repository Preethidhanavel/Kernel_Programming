#include <linux/kernel.h>   // Needed for kernel macros like printk
#include <linux/module.h>   // Needed for all kernel modules

MODULE_LICENSE("GPL");      // Declare the license type 

// Module initialization function
static int __init test_init(void)
{
    int i;

    // Loop to print messages using printk_once
    for (i = 0; i < 20; i++) {
        // printk_once ensures each unique message is printed only once
        printk_once(KERN_INFO "Message %d\n", i);
    }

    return 0;  // Return 0 indicates successful initialization
}

// Module cleanup function
static void __exit test_exit(void)
{
    printk(KERN_INFO "Exiting module\n");  // Print message when module is removed
}

// Macros to register init and exit functions
module_init(test_init);
module_exit(test_exit);
