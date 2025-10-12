#include <linux/kernel.h>   // Needed for KERN_INFO and kernel functions
#include <linux/module.h>   // Needed for all modules (module_init, module_exit, etc.)

MODULE_LICENSE("GPL");      // License type, prevents kernel from being marked as tainted

// Declare external function 'add' which is defined in another module/file
extern int add(int, int);

// Function that runs when module is loaded
static int fun_init(void)
{
    int x = add(5, 6);   // Call external function 'add' with 5 and 6
    printk(KERN_INFO "in init %s sum is:%d\n", __func__, x);  
    // Print function name (__func__) and result to kernel log
    return 0;            // Return success
}

// Function that runs when module is unloaded
static void fun_exit(void)
{
    printk(KERN_INFO "in exit %s\n", __func__);  
    // Print function name to kernel log
}

// Register init and exit functions
module_init(fun_init);   // Called when module is inserted
module_exit(fun_exit);   // Called when module is removed
