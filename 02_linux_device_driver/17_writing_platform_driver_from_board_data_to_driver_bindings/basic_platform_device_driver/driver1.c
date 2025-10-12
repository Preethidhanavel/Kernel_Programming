#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Simple platform driver");

// Probe function: called when matching device is found
static int my_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "[Platform-driver] Probe called for device: %s\n", pdev->name);
    return 0;   // success
}

// Remove function: called when device is removed/unregistered
static int my_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "[Platform-driver] Remove called for device: %s\n", pdev->name);
    return 0;
}

// Define platform driver structure
static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "platformDev1",   // Must match device name for binding
        .owner = THIS_MODULE,     // Kernel module ownership
    },
    .probe = my_probe,            // Called when device registers
    .remove = my_remove,          // Called when device unregisters
};

// Init function: register the driver with kernel
static int __init pdrv_init(void)
{
    printk(KERN_INFO "Platform driver loaded\n");
    return platform_driver_register(&my_platform_driver);
}

// Exit function: unregister driver
static void __exit pdrv_exit(void)
{
    platform_driver_unregister(&my_platform_driver);
    printk(KERN_INFO "Platform driver unloaded\n");
}

// Register init/exit functions
module_init(pdrv_init);
module_exit(pdrv_exit);
