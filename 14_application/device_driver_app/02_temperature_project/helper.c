#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/string.h>

//-----------------------------------------------------------
// This module acts as a helper provider for temperature value.
// Another kernel module can call get_temp_val() using EXPORT_SYMBOL()
//-----------------------------------------------------------

struct class *cl;
struct device *device_node;
dev_t dev_num;
struct cdev my_dev;

// Default temperature value (can be overridden using module parameter)
int temp = 20;

// Making the variable configurable from userspace using module insertion argument
// Example: insmod helper.ko temp=45
module_param(temp, int, 0644);

//-----------------------------------------------------------
// Function to return temperature value to other kernel modules
//-----------------------------------------------------------
int get_temp_val(void)
{
	return temp;
}

// Exporting symbol so that other modules can use get_temp_val()
EXPORT_SYMBOL(get_temp_val);

//-----------------------------------------------------------
// Module initialization function
//-----------------------------------------------------------
static int __init temp_init(void)
{
	pr_info("Helper module loaded successfully\n");
	pr_info("Initial temperature value: %d\n", temp);
	return 0;
}

//-----------------------------------------------------------
// Module cleanup function
//-----------------------------------------------------------
static void __exit temp_exit(void)
{
	printk(KERN_INFO "Helper module unloaded\n");
}

// Register init and exit functions
module_init(temp_init);
module_exit(temp_exit);

//-----------------------------------------------------------
// Module metadata
//-----------------------------------------------------------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Preethi");
MODULE_DESCRIPTION("A helper module exporting temperature symbol");
