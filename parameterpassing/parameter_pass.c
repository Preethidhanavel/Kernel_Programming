#include<linux/kernel.h>   // For printk() and KERN_INFO
#include<linux/module.h>   // For module related macros 

MODULE_LICENSE("GPL");     // License declaration

// Declare parameters that can be passed when inserting the module
char *name = "Embeddedd";  // Default string value
int count = 1;             // Default integer value

// Register 'name' as a module parameter
module_param(name, charp, S_IRUGO);

// Register 'count' as a module parameter
module_param(count, int, S_IRUGO);

// Initialization function (called when module is loaded)
static int test_arguments_init(void)
{
    int i;
    printk(KERN_INFO "%s: In init\n", __func__);           // Print init message
    printk(KERN_INFO "%s: count:%d\n", __func__, count);   // Print count value
    
    // Print "Hi <name>" 'count' times
    for (i = 0; i < count; i++)
        printk(KERN_INFO "%s: Hi %s\n", __func__, name);

    return 0; // success
}

// Exit function (called when module is removed)
static void test_arguments_exit(void)
{
    printk(KERN_INFO "%s: In exit\n", __func__); // Print exit message
}

// Register init and exit functions
module_init(test_arguments_init);
module_exit(test_arguments_exit);
