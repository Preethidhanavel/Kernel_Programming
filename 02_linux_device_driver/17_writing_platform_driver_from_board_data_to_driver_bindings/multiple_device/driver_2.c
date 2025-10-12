#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Platform driver handling two devices");

// --- Probe function ---
// Called when a matching platform device is found
static int my_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "[driver] Probe called for device: %s\n", pdev->name);
    return 0;
}

// --- Remove function ---
// Called when the device is removed/unregistered
static int my_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "[driver] Remove called for device: %s\n", pdev->name);
    return 0;
}

// --- Platform driver structure ---
static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "mydev",             // Base name (we override matching later)
        .owner = THIS_MODULE,
        .of_match_table = NULL,      // Not using Device Tree
    },
    .probe = my_probe,
    .remove = my_remove,
};

// --- Custom match function ---
// Default platform bus match checks only driver->name == device->name.
// Here, we extend it to accept both "mydev0" and "mydev1".
static int my_platform_match(struct device *dev, struct device_driver *drv)
{
    struct platform_device *pdev = to_platform_device(dev);

    if (!strcmp(pdev->name, "mydev0") || !strcmp(pdev->name, "mydev1"))
        return 1;  // Match success

    return 0;      // No match
}

// Platform bus is global, so we override its match function
extern struct bus_type platform_bus_type;

// --- Module init ---
static int __init pdriver_init(void)
{
    printk(KERN_INFO "[driver] Platform driver loaded\n");

    // Override default match function with our custom one
    platform_bus_type.match = my_platform_match;

    // Register driver with platform bus
    return platform_driver_register(&my_platform_driver);
}

// --- Module exit ---
static void __exit pdriver_exit(void)
{
    // Unregister driver when module is removed
    platform_driver_unregister(&my_platform_driver);
    printk(KERN_INFO "[driver] Platform driver unloaded\n");
}

// Register init and exit functions
module_init(pdriver_init);
module_exit(pdriver_exit);
