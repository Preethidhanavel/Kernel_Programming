#include <linux/kernel.h>   // Core kernel header file 
#include <linux/module.h>   // Needed for all kernel modules 

// Declare the license for the module
// "GPL" means it's licensed under GNU General Public License.
MODULE_LICENSE("GPL");

// Initialization function: called when module is loaded using insmod
static int my_init(void)
{
    // Print message to kernel log (use dmesg to view)
    printk(KERN_INFO "in %s function\n", __func__);
    return 0;   // Return 0 means successful loading
}

// Cleanup function: called when module is removed using rmmod
static void my_exit(void)
{
    // Print message to kernel log
    printk(KERN_INFO "in %s function\n", __func__);
}

// Register init and exit functions with kernel
module_init(my_init);   // Executes when module is inserted
module_exit(my_exit);   // Executes when module is removed
