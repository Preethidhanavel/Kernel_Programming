#include <linux/kernel.h>   // Needed for KERN_INFO and printk
#include <linux/module.h>   // Needed for all kernel modules

MODULE_LICENSE("abc");      // License type (use "GPL" normally to avoid warnings)


// Initialization function 
static int fun_init(void)
{
	printk(KERN_INFO "in init function\n");  // Print message to kernel log
	return 0;   // Return 0 means success
}

// Exit function 
static void fun_exit(void)
{
	printk(KERN_INFO "in exit function\n");  // Print message to kernel log
}

// Macros to register init and exit functions with the kernel
module_init(fun_init);  
module_exit(fun_exit);
