#include <linux/kernel.h>   // For printk() and KERN_INFO
#include <linux/module.h>   // For module macros

// Declare license type as "proprietary" (not GPL-compatible)
MODULE_LICENSE("proprietary");

// Declare external function prototype (to be defined in another module/file)
extern int add(int, int);

// Function that runs when module is loaded
static int fun_init(void)
{
    int x = add(5, 6);  // Call external add() function
    printk(KERN_INFO "in init %s sum is:%d\n", __func__, x);  // Print result
    return 0;   // Successful load
}

// Function that runs when module is removed
static void fun_exit(void)
{
    printk(KERN_INFO "in exit %s\n", __func__);  // Print exit message
}

// Register entry and exit points of the module
module_init(fun_init);
module_exit(fun_exit);
