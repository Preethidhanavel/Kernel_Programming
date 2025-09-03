#include <linux/kernel.h>   // For printk, pr_info, print_hex_dump
#include <linux/module.h>   // For module_init, module_exit, MODULE_LICENSE

// Declare license type 
MODULE_LICENSE("GPL");

// Initialization function 
static int hello_init(void)
{
    // Define a character array (string)
    char c[] = "hello world";

    // print_hex_dump() prints data in hex + ASCII format
    print_hex_dump(KERN_ALERT, "c: ",DUMP_PREFIX_ADDRESS, 16, 1,c, sizeof(c), 1);

    return 0;   // Successful load
}

// Cleanup function
static void hello_exit(void)
{
    printk("in exit\n");
}

// Register init and exit functions with the kernel
module_init(hello_init);   // Called on insmod
module_exit(hello_exit);   // Called on rmmod
