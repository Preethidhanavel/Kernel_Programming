#include <linux/kernel.h>   // For kernel info functions like printk
#include <linux/module.h>   // For all kernel module macros (module_init, module_exit, etc.)
#include "variable.h"       //  custom header file where 'struct test' and 'foo' are declared

MODULE_LICENSE("GPL");      // License type (GPL) to avoid kernel taint warnings

// Define the variable 'foo' (declared as extern in variable.h)
struct test foo;

// Export 'foo' so other kernel modules can access it
EXPORT_SYMBOL(foo);

// Initialization function (called when module is inserted with insmod)
static int test_export_init(void)
{
    // Initialize members of 'foo'
    foo.a = 1;
    foo.b = 2;
    foo.c = 'a';

    // Print message to kernel log (dmesg)
    printk(KERN_INFO "%s: in init %d  %d %c\n", __func__, foo.a, foo.b, foo.c);
    return 0;   // Return 0 means success
}

// Exit function (called when module is removed with rmmod)
static void test_export_exit(void)
{
    printk(KERN_INFO "%s: in exit\n", __func__);
}

// Register the above init and exit functions with the kernel
module_init(test_export_init);
module_exit(test_export_exit);
