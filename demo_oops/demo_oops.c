#include <linux/kernel.h>   // For printk 
#include <linux/module.h>   // For module_init, module_exit, MODULE_LICENSE, etc.

// Initialization function - runs when the module is loaded 
static int init_oops(void)
{
    // Print the function name
    printk(KERN_INFO "%s: in init\n", __func__);

    // This line deliberately causes a kernel OOPS
    // It tries to write 'a' into memory address 0x12
    // That is an invalid (non-mapped) memory address in kernel space
    // -> Leads to "Oops" error in kernel log (but may not fully crash system)
    *(int *)0x12 = 'a';

    return 0;   // This may not execute if OOPS occurs
}

// Cleanup function - runs when the module is removed
static void exit_oops(void)
{
    // Print the function name when exiting
    printk(KERN_INFO "%s \n", __func__);
}

// Register init and exit functions with kernel
module_init(init_oops);   // Called on module load
module_exit(exit_oops);   // Called on module unload
