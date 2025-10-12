#include <linux/kernel.h>   // Needed for KERN_INFO and printk
#include <linux/module.h>   // Needed for all kernel modules

MODULE_LICENSE("GPL");      // Specify module license 

// Define module parameters
char *name = "Embeddedd";   // Default string parameter
int count = 1;              // Default integer parameter

// Register module parameters
module_param(name, charp, 0);         // 'name' is a string (char pointer), no permission flags
// module_param(name, charp, S_IRUGO); // Alternative: read-only from sysfs
module_param(count, int, S_IRUGO);    // 'count' is integer, readable from sysfs

// Module initialization function
static int test_arguments_init(void)
{
    int i;
    printk(KERN_INFO "%s: In init\n", __func__);     // Print when module loads
    printk(KERN_INFO "%s: count:%d\n", __func__, count);

    // Print message 'count' times
    for (i = 0; i < count; i++)
        printk(KERN_INFO "%s: Hi %s\n", __func__, name);

    return 0;   // Success
}

// Module cleanup function
static void test_arguments_exit(void)
{
    printk(KERN_INFO "%s: In exit\n", __func__);    // Print when module unloads
}

// Register init and exit functions
module_init(test_arguments_init);
module_exit(test_arguments_exit);
