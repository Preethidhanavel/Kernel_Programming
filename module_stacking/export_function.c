#include <linux/kernel.h>   // Needed for KERN_INFO and kernel functions
#include <linux/module.h>   // Needed for module macros and functions

MODULE_LICENSE("GPL");      // License type, required to avoid "tainted kernel"

// Define a function 'add' that adds two integers
int add(int a, int b)
{
    printk(KERN_INFO "%s: sum:%d\n", __func__, a + b);  
    // Print function name (__func__) and result to kernel log
    return a + b;  // Return the sum
}

// Function that runs when module is loaded
static int test_export_init(void)
{
    printk(KERN_INFO "%s: in init\n", __func__);  
    // Print message when module is initialized
    return 0;  
}

// Function that runs when module is unloaded
static void test_export_exit(void)
{
    printk(KERN_INFO "%s: in exit\n", __func__);  
    // Print message when module is removed
}

// Export 'add' function so it can be used by other modules
EXPORT_SYMBOL_GPL(add);

// Register init and exit functions
module_init(test_export_init);   // Called on module load
module_exit(test_export_exit);   // Called on module unload
