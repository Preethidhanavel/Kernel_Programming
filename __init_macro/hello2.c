#include <linux/kernel.h>   // Contains kernel functions like printk
#include <linux/module.h>   // Needed for all kernel modules 

// License declaration 
// "GPL" means module is open-source under GNU Public License
MODULE_LICENSE("GPL");

// Initialization function 
static int __init my_init(void)
{
    // Print message to kernel log buffer 
    printk(KERN_INFO "in %s function\n", __func__);
    return 0;   // 0 means successful load
}

// Exit/cleanup function 
static void __exit my_exit(void)
{
    // Print message when module is removed
    printk(KERN_INFO "in %s function\n", __func__);
}

// Macros to register init and exit functions
module_init(my_init);   // Called on insmod
module_exit(my_exit);   // Called on rmmod
