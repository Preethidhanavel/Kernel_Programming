#include <linux/kernel.h>  // Needed for printk()
#include <linux/module.h>  // Needed for all modules

MODULE_LICENSE("GPL");     // Correct license type 

// Module initialization function
static int init_oops(void)
{
#ifdef DEBUG
    // Print message only if DEBUG is defined in Makefile or source
    printk(KERN_INFO "%s: in init\n", __func__);
#endif
    return 0;  // Success
}

// Module cleanup function
static void exit_oops(void)
{
#ifdef DEBUG
    // Print message only if DEBUG is defined
    printk(KERN_INFO "%s\n", __func__);
#endif
}

// Register initialization and cleanup functions
module_init(init_oops);
module_exit(exit_oops);
