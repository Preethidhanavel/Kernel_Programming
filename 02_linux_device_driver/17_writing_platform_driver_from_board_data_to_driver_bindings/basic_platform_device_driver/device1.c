#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Simulated platform device");

// Define a platform device structure
static struct platform_device my_pdev = {
    .name = "platformDev1",  // Device name (driver will bind using this)
    .id = -1,                // -1 means no multiple instances (single device)
};

// Module init function
static int __init pdev_init(void)
{
    printk(KERN_INFO "Registering platform device...\n");
    // Register the platform device with the kernel
    return platform_device_register(&my_pdev);
}

// Module exit function
static void __exit pdev_exit(void)
{
    printk(KERN_INFO "Unregistering platform device...\n");
    // Unregister (remove) the platform device
    platform_device_unregister(&my_pdev);
}

// Register init and exit functions
module_init(pdev_init);
module_exit(pdev_exit);
