#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Platform driver using device tree");

// Example structure (not used directly here, but can store DT data)
struct mydt_data {
    int id;
    const char *label;
};

// Probe function - runs when a matching DT node + driver is found
static int dt_probe(struct platform_device *pdev)
{
    const struct of_device_id *match;   // Pointer to matching DT entry
    struct device *dev = &pdev->dev;    // Get Linux device struct

    // Find matching entry in "dt_ids" using device's of_node
    match = of_match_device(dt_ids, dev);
    if (!match) {
        dev_err(dev, "No compatible match found\n");
        return -ENODEV;
    }

    // Print which compatible string matched
    dev_info(dev, "Matched compatible string: %s\n", match->compatible);

    // Get GPIO number from DT property "gpios" (first index = 0)
    int gpio = of_get_named_gpio(dev->of_node, "gpios", 0);

    // Get IRQ number assigned to this platform device (index 0)
    int irq  = platform_get_irq(pdev, 0);

    // Print out the values
    dev_info(dev, "Device %s: gpio=%d irq=%d\n",
             match->compatible, gpio, irq);

    return 0; // Success
}

// Remove function - called when device is unbound/removed
static void dt_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Device removed\n");
    return;
}

// Match table - defines which DT "compatible" strings this driver supports
static const struct of_device_id dt_ids[] = {
    { .compatible = "myvendor,devtype1" }, // device type 1
    { .compatible = "myvendor,devtype2" }, // device type 2
    { }
};
MODULE_DEVICE_TABLE(of, dt_ids);  // Export table to user space (modinfo)

// Define platform driver structure
static struct platform_driver dt_driver = {
    .probe = dt_probe,     // Called on device match
    .remove = dt_remove,   // Called on device removal
    .driver = {
        .name = "mydevice",        // Driver name (shows in sysfs)
        .of_match_table = dt_ids,  // Match table for DT binding
    },
};

// Register platform driver when module loads, unregister on exit
module_platform_driver(dt_driver);
