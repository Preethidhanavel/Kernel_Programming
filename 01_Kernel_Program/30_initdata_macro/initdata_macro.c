#include <linux/kernel.h>   // Needed for kernel info/debug macros
#include <linux/module.h>   // Needed for all kernel modules

MODULE_LICENSE("GPL");      // License type for the module 

// Global buffer (character array) initialized with "hello"
char buf[] = "hello";

// Module initialization function 
static int my_init(void)
{
    // Print message to kernel log with function name and buffer content
    printk(KERN_INFO "in %s function buffer:%s\n", __func__, buf);
    return 0;   // Return 0 indicates successful loading
}

// Module cleanup function 
static void my_exit(void)
{
    // Print exit message to kernel log
    printk(KERN_INFO "exit function\n");
}

// Register init and exit functions
module_init(my_init);
module_exit(my_exit);
