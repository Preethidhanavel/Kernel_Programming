#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Platform driver handling two devices");

static int my_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "[driver] Probe called for device: %s\n", pdev->name);
    return 0;
}

static int my_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "[driver] Remove called for device: %s\n", pdev->name);
    return 0;
}

static struct platform_driver my_platform_driver = {
    .driver = {
        .name = "mydev",   
        .owner = THIS_MODULE,
        .of_match_table = NULL, // match manually 
    },
    .probe = my_probe,
    .remove = my_remove,
};

/* Custom match function override */
static int my_platform_match(struct device *dev, struct device_driver *drv)
{
    struct platform_device *pdev = to_platform_device(dev);

    if (!strcmp(pdev->name, "mydev0") || !strcmp(pdev->name, "mydev1"))
        return 1;

    return 0;
}

extern struct bus_type platform_bus_type;

static int __init pdriver_init(void)
{
    printk(KERN_INFO "[driver] Platform driver loaded\n");

    /* override default match with ours */
    platform_bus_type.match = my_platform_match;

    return platform_driver_register(&my_platform_driver);
}

static void __exit pdriver_exit(void)
{
    platform_driver_unregister(&my_platform_driver);
    printk(KERN_INFO "[driver] Platform driver unloaded\n");
}

module_init(pdriver_init);
module_exit(pdriver_exit);

