#include <linux/kernel.h>   // Needed for printk()
#include <linux/module.h>   // Needed for all modules

MODULE_LICENSE("GPL");      // Specify license type 

// Initialization function 
static int fun_init(void)
{
    // printk with a special prefix "\001""4"
    // "\001" is SOH (Start of Header) control character
    // "4" indicates log level = KERN_WARNING
    printk("\001""4""in init\n");
    return 0;   // Success
}

// Exit function 
static void fun_exit(void)
{
    // "\001""6" indicates log level = KERN_INFO
    printk("\001""6""in exit\n");
}

// Register init and exit functions
module_init(fun_init);
module_exit(fun_exit);
