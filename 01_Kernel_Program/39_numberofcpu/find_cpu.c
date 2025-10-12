#include <linux/kernel.h>   // Needed for printk()/pr_info()
#include <linux/module.h>   // Needed for all modules
#include <linux/cpumask.h>  // Needed for num_online_cpus() and CPU-related macros

MODULE_LICENSE("GPL");      // Specify license type

// Module initialization function
static int fun_init(void)
{
    // Print the number of CPUs currently online
    pr_info("number of CPU: %d\n", num_online_cpus());
    return 0;  // Return 0 indicates successful initialization
}

// Module cleanup function
static void fun_exit(void)
{
    printk("exit function\n");  // Log module removal
}

// Register initialization and cleanup functions
module_init(fun_init);
module_exit(fun_exit);
