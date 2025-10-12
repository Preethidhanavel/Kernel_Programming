#include<linux/kernel.h>   // For printk() and KERN_INFO
#include<linux/module.h>   // For module macros 

// Declare module license as GPL 
MODULE_LICENSE("GPL");

// init_module() is the entry function 
int init_module(void)
{
    // Print a message in the kernel log with function name
    printk(KERN_INFO "%s: In init\n", __func__);
    return 0;   // return 0 = success
}

// cleanup_module() is the exit function 
void cleanup_module(void)
{
    // Print a message in the kernel log with function name 
    printk(KERN_INFO "%s: In exit\n", __func__);
}
