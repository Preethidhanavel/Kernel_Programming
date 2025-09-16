#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Register two platform devices");

// Define first platform device (mydev0)
static struct platform_device dev0 = {
    .name = "mydev0",   // Device name (must match with driver's .name)
    .id   = -1,         // -1 means no multiple instances
};

// Define second platform device (mydev1)
static struct platform_device dev1 = {
    .name = "mydev1",   // Device name (must match with driver)
    .id   = -1,
};

// Module initialization function
static int __init pdevs_init(void)
{
    printk(KERN_INFO "[pdevs] Registering dev0 and dev1...\n");

    // Register both devices with the kernel
    platform_device_register(&dev0);
    platform_device_register(&dev1);

    return 0;  // success
}

// Module cleanup function
static void __exit pdevs_exit(void)
{
    printk(KERN_INFO "[pdevs] Unregistering dev0 and dev1...\n");

    // Unregister devices during module removal
    platform_device_unregister(&dev0);
    platform_device_unregister(&dev1);
}

// Register module entry and exit points
module_init(pdevs_init);
module_exit(pdevs_exit);
