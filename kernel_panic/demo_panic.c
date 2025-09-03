#include <linux/kernel.h>   // Needed for kernel functions like printk and panic
#include <linux/module.h>   // Needed for module macros

MODULE_LICENSE("GPL");      // Declare license as GPL to avoid tainting the kernel

// Initialization function
static int panic_init(void)
{
    // Print a log message when entering the init function
    printk(KERN_INFO "%s: in init\n", __func__);

    // Trigger a kernel panic with a custom message
    panic("hello kernel - panic attack\n");

    return 0;   // This line will never be reached because panic() halts the system
}

// Exit function 
static void panic_exit(void)
{
    // Print a log message when exiting 
    printk(KERN_INFO "%s: in exit\n", __func__);
}

// Register the initialization and cleanup functions with the kernel
module_init(panic_init);
module_exit(panic_exit);
