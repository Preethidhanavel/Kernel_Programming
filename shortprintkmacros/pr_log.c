#include <linux/kernel.h>  // Needed for kernel macros like pr_info, pr_warn
#include <linux/module.h>  // Needed for all kernel modules

MODULE_LICENSE("GPL");     // Declare the license type

// Module initialization function
static int fun_init(void)
{
    // Print informational message when module is inserted
    pr_info("%s: In init\n", __func__);
    return 0;  // Return 0 indicates successful initialization
}

// Module cleanup function
static void fun_exit(void)
{
    // Print warning message when module is removed
    pr_warn("%s: in exit\n", __func__);
}

// Macros to register init and exit functions
module_init(fun_init);
module_exit(fun_exit);
