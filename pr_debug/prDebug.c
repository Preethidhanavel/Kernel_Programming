#include <linux/kernel.h>  // Needed for printk() and pr_debug()
#include <linux/module.h>  // Needed for all modules

MODULE_LICENSE("GPL");     // Specify license type 

// Module initialization function
static int fun_init(void)
{
    printk(KERN_DEBUG "in init 1\n");                 // Standard debug-level message
    pr_debug("%s: In init2\n", __func__);            // Another debug-level message 
    return 0;                                        // Return 0 indicates successful init
}

// Module cleanup function
static void fun_exit(void)
{
    pr_debug("%s: in exit2\n", __func__);            // Debug-level message on module removal
    pr_warn("in exit1\n");                           // Warning-level message 
}

// Register initialization and cleanup functions
module_init(fun_init);
module_exit(fun_exit);
