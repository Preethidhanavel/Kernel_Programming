#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Platform driver accessing platform_data");

// --- Platform data structure ---
// Must match the structure defined in the platform device
struct my_pdata {
    int id;
    char label[20];
};

// --- Probe function ---
// Called when a matching platform device is found
static int my_probe(struct platform_device *pdev)
{
    // Retrieve platform-specific data
    struct my_pdata *pdata = dev_get_platdata(&pdev->dev);

    // Print the platform data
    printk(KERN_INFO "data: id-%d, label: %s\n", pdata->id, pdata->label);

    printk(KERN_INFO "[Platform-driver] Probe called for device: %s\n", pdev->name);
    return 0; // Success
}

// --- Remove function ---
// Called when the device is removed or driver is unloaded
static int my_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "[Platform-driver] Remove called for device: %s\n", pdev->name);
    return 0;
}

// --- Platform driver structure ---
static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "platformDev1",  // Must match the device name
        .owner = THIS_MODULE,
    },
    .probe = my_probe,
    .remove = my_remove,
};

// --- Module init ---
static int __init pdrv_init(void)
{
    printk(KERN_INFO "Platform driver loaded\n");

    // Register the driver with the platform bus
    return platform_driver_register(&my_platform_driver);
}

// --- Module exit ---
static void __exit pdrv_exit(void)
{
    // Unregister the driver
    platform_driver_unregister(&my_platform_driver);
    printk(KERN_INFO "Platform driver unloaded\n");
}

// Register init and exit functions
module_init(pdrv_init);
module_exit(pdrv_exit);
