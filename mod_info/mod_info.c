#include<linux/kernel.h>   // Needed for kernel functions like printk
#include<linux/module.h>   // Needed for module macros and THIS_MODULE

// Initialization function - runs when the module is loaded
static int fun_init(void)
{
    printk(KERN_INFO "in init function\n");   // Print message when module loads
    printk(KERN_INFO "name:%s\n", THIS_MODULE->name);      // Print module name
    printk(KERN_INFO "version:%s\n", THIS_MODULE->version); // Print module version

    return 0;   // Return success
}

// Cleanup function - runs when the module is removed
static void fun_exit(void)
{
    printk(KERN_INFO "in exit function\n");   // Print message when module unloads
}

// Register module entry and exit points
module_init(fun_init);  
module_exit(fun_exit);  

// Extra metadata about the module
MODULE_INFO(name, "module");     // Custom info field: module name
MODULE_INFO(OS, "linux");        // Custom info field: operating system
MODULE_VERSION("1.0");           // Define module version
MODULE_LICENSE("GPL");           // Define license (GPL allows symbol exports)
