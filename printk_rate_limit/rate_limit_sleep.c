#include <linux/kernel.h>      // Needed for kernel macros like printk
#include <linux/module.h>      // Needed for all kernel modules
#include <linux/ratelimit.h>   // Needed for printk_ratelimit()
#include <linux/delay.h>       // Needed for msleep()

MODULE_LICENSE("GPL");         // Declare the license type 

// Module initialization function
static int __init test_init(void)
{
    int i;

    // Loop to print messages with rate limiting
    for (i = 0; i < 50; i++) {
        // Check if it's okay to print without exceeding rate limit
        if (printk_ratelimit())
            printk(KERN_INFO "Message %d\n", i);  // Print message if allowed
        else
        {
            pr_info("sleeping for 5 seconds\n");  // Inform that module is sleeping
            msleep(5000);                          // Sleep for 5 seconds to avoid flooding
        }
    }

    return 0;  // Return 0 indicates successful initialization
}

// Module cleanup function
static void __exit test_exit(void)
{
    printk(KERN_INFO "Exiting module\n");  // Print message when module is removed
}

// Macros to register init and exit functions
module_init(test_init);
module_exit(test_exit);
