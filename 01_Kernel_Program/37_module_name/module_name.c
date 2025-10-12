#include <linux/kernel.h>   // Needed for kernel functions like printk
#include <linux/module.h>   // Needed for module macros

MODULE_LICENSE("GPL");       // License type 

// Store the module name as a string using __stringify(KBUILD_MODNAME)
// KBUILD_MODNAME is automatically set to the module's name by the build system
char *name = __stringify(KBUILD_MODNAME);

// Module initialization function 
static int hello_init(void)
{
    printk("%s\n", __func__);         // Print current function name
    printk("modulename: %s\n", name); // Print the module name
    return 0;                         // Return success
}

// Module cleanup function 
static void hello_exit(void)
{
    printk("in exit\n");              // Print exit message
}

// Register init and exit functions
module_init(hello_init);
module_exit(hello_exit);
