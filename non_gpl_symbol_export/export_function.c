#include <linux/kernel.h>   // For printk() and KERN_INFO
#include <linux/module.h>   // For module-related macros

// License declaration (GPL allows symbol export with EXPORT_SYMBOL_GPL)
MODULE_LICENSE("GPL");

// Function definition (adds two integers)
int add(int a, int b)
{
    printk(KERN_INFO "sum:%d\n", a + b);  // Print sum in kernel log
    return a + b;                         // Return result
}

// Function executed when module is loaded
static int test_export_init(void)
{
    printk(KERN_INFO "%s : in init\n", __func__);  // Log init message
    return 0;   // Successful load
}

// Function executed when module is removed
static void test_export_exit(void)
{
    printk(KERN_INFO "%s : in exit\n", __func__);  // Log exit message
}

// Export the function 'add' so other modules can use it
EXPORT_SYMBOL_GPL(add);

// Register init and exit functions
module_init(test_export_init);
module_exit(test_export_exit);
