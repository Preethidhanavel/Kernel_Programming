#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO LED platform driver");

// Global handle for the LED GPIO
static struct gpio_desc *led_gpiod;

// Probe function - called when a matching DT node is found
static int gpioled_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    // Request the GPIO from device tree (default direction = output low)
    led_gpiod = gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(led_gpiod);
    }

    dev_info(dev, "GPIO LED driver bound successfully\n");

    // Turn the LED ON
    gpiod_set_value(led_gpiod, 1);
    dev_info(dev, "LED turned ON\n");

    return 0; // success
}

// Remove function - called when driver is unloaded or device is removed
static int gpioled_remove(struct platform_device *pdev)
{
    gpiod_set_value(led_gpiod, 0);  // Turn off LED
    gpiod_put(led_gpiod);           // Release the GPIO descriptor
    dev_info(&pdev->dev, "GPIO LED driver removed\n");
    return 0;
}

// Device tree match table - matches "compatible" string in DT
static const struct of_device_id gpioled_of_ids[] = {
    { .compatible = "myvendor,gpioled" }, // must match DT node
    { }
};
MODULE_DEVICE_TABLE(of, gpioled_of_ids);

// Platform driver structure
static struct platform_driver gpioled_driver = {
    .driver = {
        .name = "gpioled",                 // Driver name
        .of_match_table = gpioled_of_ids,  // Match table for DT binding
    },
    .probe = gpioled_probe,   // Called when device is found
    .remove = gpioled_remove, // Called when device is removed
};

// Register platform driver when module loads, unregister when it unloads
module_platform_driver(gpioled_driver);
