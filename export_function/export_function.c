#include <linux/kernel.h>   // Needed for KERN_INFO and printk
#include <linux/module.h>   // Needed for all kernel modules
#include <linux/jiffies.h>  // Provides access to jiffies variable

MODULE_LICENSE("GPL");       // License type 

// Prints the current value of jiffies (number of ticks since boot).
void print_jiffies(void)
{
    printk(KERN_INFO "%s: jiffies: %ld\n", __func__, jiffies);
}

//init  function
static int test_export_init(void)
{
    printk(KERN_INFO "%s: in init\n", __func__);
    return 0;   // Return 0 for successful load
}

//exit function
static void test_export_exit(void)
{
    printk(KERN_INFO "%s: in exit\n", __func__);
}

//Exports the function 'print_jiffies' so other GPL kernel modules can use it 
EXPORT_SYMBOL_GPL(print_jiffies);

// Register the init and exit functions
module_init(test_export_init);
module_exit(test_export_exit);
