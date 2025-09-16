#include <linux/init.h>      // For module init/exit macros
#include <linux/module.h>    // For all kernel module related macros

MODULE_LICENSE("GPL");                  // License type
MODULE_AUTHOR("PREETHI");                   // Author info
MODULE_DESCRIPTION("Core module exporting log function"); // Module description

// Function that prints a log message to kernel log
void core_log_message(const char *msg)
{
    printk(KERN_INFO "[core_driver] Log: %s\n", msg);
}

// Export the function so that other kernel modules can use it
EXPORT_SYMBOL(core_log_message);

// Module initialization function (runs when module is loaded)
static int __init core_init(void)
{
    printk(KERN_INFO "core_driver loaded\n");
    return 0;   // Return 0 means success
}

// Module cleanup function (runs when module is unloaded)
static void __exit core_exit(void)
{
    printk(KERN_INFO "core_driver unloaded\n");
}

// Register init and exit functions with kernel
module_init(core_init);
module_exit(core_exit);
