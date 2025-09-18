#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Platform driver using device tree");

// Structure to hold device-specific data read from DT
struct mydt_data {
    int id;
    const char *label;
};

// Probe function - called when a matching DT node is found
static int dt_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;       // Get device pointer
    struct device_node *np = dev->of_node; // Get DT node of this device
    struct mydt_data data;                 // Local structure to hold DT values

    if (!np) {
        dev_err(dev, "No device tree node found\n");
        return -EINVAL;
    }

    // Read "id" property from DT, if not present set default = -1
    if (of_property_read_u32(np, "id", &data.id))
        data.id = -1;

    // Read "label" property from DT, if not present set default = "unknown"
    if (of_property_read_string(np, "label", &data.label))
        data.label = "unknown";

    // Print the values read from DT
    dev_info(dev, "Probed via DT: id=%d, label=%s\n", data.id, data.label);
    return 0;
}

// Remove function - called when device is removed/unbound
static void dt_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "Device removed\n");
    return;
}

// Match table - associates this driver with DT "compatible" string
static const struct of_device_id dt_ids[] = {
    { .compatible = "myvendor,mydevice" }, // must match DT node
    { }
};
MODULE_DEVICE_TABLE(of, dt_ids);

// Platform driver structure
static struct platform_driver dt_driver = {
    .probe = dt_probe,     // Called on match
    .remove = dt_remove,   // Called on device removal
    .driver = {
        .name = "mydevice",         // Driver name
        .of_match_table = dt_ids,   // Link to DT match table
    },
};

// Registers platform driver with module init/exit
module_platform_driver(dt_driver);
