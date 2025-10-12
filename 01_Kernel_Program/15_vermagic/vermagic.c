#include <linux/kernel.h>    // For printk(), pr_info()
#include <linux/module.h>    // For all kernel module definitions
#include <linux/vermagic.h>  // For VERMAGIC_STRING (kernel version magic string)

// Initialization function (runs when module is loaded using insmod)
static int fun_init(void)
{
    // Print the vermagic string of the currently running kernel
    pr_info("vermagic string = " VERMAGIC_STRING "\n");
    return 0;   // 0 means success
}

// Exit function (runs when module is removed using rmmod)
static void fun_exit(void)
{
    printk(KERN_INFO "In exit\n");  // Log exit message
}

// Register init and exit functions with the kernel
module_init(fun_init);
module_exit(fun_exit);

// Add custom module information
MODULE_INFO(vermagic, "123");   // Overwrite module's vermagic with "123"
MODULE_LICENSE("GPL");          // Declare license (GPL) to avoid kernel taint
