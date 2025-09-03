#include <linux/kernel.h>   // For printk 
#include <linux/module.h>   // For module_init, module_exit, MODULE_LICENSE, etc.

// Initialization function - runs when module is loaded 
static int init_oops(void)
{
    // Print function name using __func__
    printk(KERN_INFO "%s: in init\n", __func__);

    // BUG() is a kernel macro that forces a kernel panic immediately
    // It is used for debugging, but will crash the system if executed
    BUG();

    return 0;   // This line will never be reached 
}

// Cleanup function - runs when module is unloaded 
static void exit_oops(void)
{
    // Print function name when exiting
    printk(KERN_INFO "%s \n", __func__);
}

// Register the init and exit functions with the kernel
module_init(init_oops);   // Called when the module is inserted
module_exit(exit_oops);   // Called when the module is removed
