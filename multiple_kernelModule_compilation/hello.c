#include <linux/init.h>       // Required for module init and exit macros
#include <linux/module.h>     // Required for all kernel modules

// Function executed when the module is loaded into the kernel
static int __init hello_init(void)
{
	printk(KERN_INFO "from hello init\n");   // Print message to kernel log
	return 0;   // Return 0 to indicate successful loading
}

// Function executed when the module is removed from the kernel
static void __exit hello_exit(void)
{
    printk(KERN_INFO "exit from hello\n");   // Print message to kernel log
}

// Register the init and exit functions
module_init(hello_init);  
module_exit(hello_exit);

// Module metadata
MODULE_LICENSE("GPL");               // License type (avoid tainting kernel)
MODULE_AUTHOR("PREETHI");            // Author information
MODULE_DESCRIPTION("first module");  // Short description of the module

