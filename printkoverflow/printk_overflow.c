#include <linux/kernel.h>  // Needed for kernel macros like printk
#include <linux/module.h>  // Needed for all kernel modules

MODULE_LICENSE("GPL");     // Declare the license type 

// Function to print numbers from 0 to 49999
void fun(void)
{
    int i;
    for (i = 0; i < 50000L; i++)
        printk(KERN_INFO "%d\n", i);  // Print each number to kernel log
}

// Module initialization function
static int fun_init(void)
{
    printk(KERN_INFO "in init\n");  // Print message when module is inserted
    fun();                            // Call the function to print numbers
    return 0;                         // Return 0 indicates successful initialization
}

// Module cleanup function
static void fun_exit(void)
{
    printk(KERN_INFO "in exit\n");   // Print message when module is removed
}

// Macros to register init and exit functions
module_init(fun_init);
module_exit(fun_exit);
