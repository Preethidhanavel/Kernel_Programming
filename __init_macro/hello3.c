#include <linux/kernel.h>   // Provides core kernel functions 
#include <linux/module.h>   // Required for all kernel modules 
#include <linux/delay.h>    // Provides delay functions like msleep

// Declare license for the module
MODULE_LICENSE("GPL");

// Initialization function 
static int my_init(void)
{
    // Print message to kernel log 
    printk(KERN_INFO "in %s function\n", __func__);

    // Sleep for 100 seconds 
    // ssleep() takes seconds as argument
    ssleep(100);

    return 0;   // Return 0 = success
}

// Exit/cleanup function 
static void my_exit(void)
{
    // Print message to kernel log
    printk(KERN_INFO "in %s function\n", __func__);
}

// Register initialization and exit functions with kernel
module_init(my_init);   // Called when module is loaded
module_exit(my_exit);   // Called when module is unloaded
