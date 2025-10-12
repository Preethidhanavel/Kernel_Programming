#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Simulated platform device with platform data");

// --- Platform data structure ---
struct my_pdata {
    int id;
    char label[20];
};

// Initialize platform data
static struct my_pdata my_data = {
    .id = 3,
    .label = "voltagenode"
};

// --- Platform device definition ---
static struct platform_device my_pdev = {
    .name = "platformDev1",   // Device name (must match driver name)
    .id = -1,                 // -1 = single instance
    .dev = {
        .platform_data = &my_data,  // Attach platform-specific data
    },
};

// --- Module init function ---
static int __init pdev_init(void)
{
    printk(KERN_INFO "Registering platform device...\n");

    // Register the platform device with kernel
    return platform_device_register(&my_pdev);
}

// --- Module exit function ---
static void __exit pdev_exit(void)
{
    printk(KERN_INFO "Unregistering platform device...\n");

    // Unregister device during module removal
    platform_device_unregister(&my_pdev);
}

// Register init and exit functions
module_init(pdev_init);
module_exit(pdev_exit);
