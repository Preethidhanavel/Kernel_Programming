#include<linux/kernel.h>   // For printk() and KERN_INFO

// Define a simple function 'func'
void func(void)
{
    // Print a message to the kernel log buffer
    // KERN_INFO : log level (informational)
    printk(KERN_INFO "Helllo linux\n");
}
