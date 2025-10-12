#include <linux/kernel.h>   // For printk 
#include <linux/module.h>   // Needed for all kernel modules 

// Declare the license type (required for kernel modules)
// "GPL" means GNU Public License
MODULE_LICENSE("GPL");

// Initialization function - called when module is loaded 
static int fun_init(void)
{
    // Print message to kernel log 
    printk("in init\n");
    return 0;   // Return 0 means success
}

// Cleanup function - called when module is removed 
static void fun_exit(void)
{
    // Print message to kernel log
    printk("in exit\n");
}

// Register the init and exit functions with the kernel
module_init(fun_init);   // Runs on module insertion
module_exit(fun_exit);   // Runs on module removal
