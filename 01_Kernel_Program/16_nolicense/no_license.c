#include <linux/kernel.h>   // For printk() and KERN_INFO
#include <linux/module.h>   // For module_init(), module_exit() etc.

// Initialization function
static int fun_init(void)
{
    printk(KERN_INFO "in init function\n");  // Print message to kernel log (dmesg)
    return 0;  // Return 0 means success
}

// Exit function 
static void fun_exit(void)
{
    printk(KERN_INFO "in exit function\n");  // Print message to kernel log
}

// Register init and exit functions with the kernel
module_init(fun_init);   // Called at module insertion
module_exit(fun_exit);   // Called at module removal
