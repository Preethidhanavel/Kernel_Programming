#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Register two platform devices");

static struct platform_device dev0 = {
    .name = "mydev0",
    .id   = -1,
};

static struct platform_device dev1 = {
    .name = "mydev1",
    .id   = -1,
};

static int __init pdevs_init(void)
{
    printk(KERN_INFO "[pdevs] Registering dev0 and dev1...\n");
    platform_device_register(&dev0);
    platform_device_register(&dev1);
    return 0;
}

static void __exit pdevs_exit(void)
{
    printk(KERN_INFO "[pdevs] Unregistering dev0 and dev1...\n");
    platform_device_unregister(&dev0);
    platform_device_unregister(&dev1);
}

module_init(pdevs_init);
module_exit(pdevs_exit);

