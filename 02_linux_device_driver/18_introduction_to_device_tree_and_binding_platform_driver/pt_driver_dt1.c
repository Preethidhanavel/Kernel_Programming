#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Platform driver using device tree");

// Custom structure (not used in this example, but can hold DT data)
struct mydt_data {
    int id;
    const char *label;
};

// Probe function - called when a matching DT node is found
static int dt_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;        // Pointer to device
    struct device_node *np = dev->of_node;  // Get device tree node for this device
    u32 gpio_val, irq_val;                  // Variables to store DT values
    int ret;

    // Read "gpio" property from DT node
    ret = of_property_read_u32(np, "gpio", &gpio_val);
    if (ret) {
        dev_err(dev, "Failed to read gpio property\n");
        return ret; // Exit if property not found
    }

    // Read "irq" property from DT node
    ret = of_property_read_u32(np, "irq", &irq_val);
    if (ret) {
        dev_err(dev, "Failed to read irq property\n");
        return ret; // Exit if property not found
    }

    // Print the values read from DT
    dev_info(dev, "Read from DT: gpio=%u irq=%u\n", gpio_val, irq_val);

    return 0; // Success
}

// Remove function - called when device is removed/unbound
static void dt_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Device removed\n");
    return;
}

// Match table - links compatible string in DT with this driver
static const struct of_device_id dt_ids[] = {
    { .compatible = "myvendor,mydevice" }, // must match DT node
    { }
};
MODULE_DEVICE_TABLE(of, dt_ids);

// Platform driver definition
static struct platform_driver dt_driver = {
    .probe = dt_probe,     // Called when DT match occurs
    .remove = dt_remove,   // Called when device is removed
    .driver = {
        .name = "mydevice",        // Driver name
        .of_match_table = dt_ids,  // DT match table
    },
};

// Register driver at module load, unregister at module unload
module_platform_driver(dt_driver);
