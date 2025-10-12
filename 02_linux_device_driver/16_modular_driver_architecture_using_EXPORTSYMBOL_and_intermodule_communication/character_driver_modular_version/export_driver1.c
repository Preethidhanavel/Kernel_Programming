#include <linux/init.h>     // For module init/exit macros
#include <linux/module.h>   // For module utilities

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Core module exporting log function and version");

// --- Function to log messages from other modules ---
void core_log_message(const char *msg)
{
    printk(KERN_INFO "[core_driver] Log: %s\n", msg);
}
// Make this function available to other modules
EXPORT_SYMBOL(core_log_message);

// --- Function to provide version string ---
const char *core_version(void)
{
    return "CoreDriver v1.0";  // Return version text
}
// Export version function so other modules can call it
EXPORT_SYMBOL(core_version);

// --- Module initialization function ---
static int __init core_init(void)
{
    printk(KERN_INFO "core_driver loaded\n");
    return 0;
}

// --- Module cleanup function ---
static void __exit core_exit(void)
{
    printk(KERN_INFO "core_driver unloaded\n");
}

module_init(core_init);   // Register init function
module_exit(core_exit);   // Register exit function
