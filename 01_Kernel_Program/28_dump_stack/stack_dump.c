#include <linux/kernel.h>   // For printk, pr_info, dump_stack
#include <linux/module.h>   // For module_init, module_exit, MODULE_LICENSE

// Initialization function 
static int my_init(void)
{
    // Print a log message before dumping the stack
    pr_info("dump stack in init\n");

    // dump_stack() prints the current call stack 
    // Useful for debugging and seeing how code execution reached here
    dump_stack();

    // Print a log message after dumping the stack
    pr_info("dump stack end\n");

    return 0;   // Return 0 means successful module load
}

// Cleanup function 
static void my_exit(void)
{
    // Simple log message on module removal
    printk("in exit\n");
}

// Register the init and exit functions with the kernel
module_init(my_init);   // Called when module is inserted
module_exit(my_exit);   // Called when module is removed

// Declare module license 
MODULE_LICENSE("GPL");
