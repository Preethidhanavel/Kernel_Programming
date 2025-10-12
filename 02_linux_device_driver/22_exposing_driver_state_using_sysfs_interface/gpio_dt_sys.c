#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/device.h>  // for device_create_file

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("GPIO LED platform driver with sysfs control");

static struct gpio_desc *led_gpiod;

/* Sysfs "store" callback: write 1 or 0 */
static ssize_t control_store(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf, size_t count)
{
    int value;

    if (kstrtoint(buf, 0, &value))   // convert string -> int
        return -EINVAL;

    if (value == 1) {
        gpiod_set_value(led_gpiod, 1);
        dev_info(dev, "LED turned ON via sysfs\n");
    } else if (value == 0) {
        gpiod_set_value(led_gpiod, 0);
        dev_info(dev, "LED turned OFF via sysfs\n");
    } else {
        dev_warn(dev, "Invalid value: use 0 or 1\n");
    }

    return count;
}

/* Optional: sysfs "show" callback for reading state */
static ssize_t control_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
    int val = gpiod_get_value(led_gpiod);
    return sprintf(buf, "%d\n", val);
}

/* Create attribute: rw (read/write) */
static DEVICE_ATTR(control, 0660, control_show, control_store);

static int gpioled_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int ret;

    led_gpiod = gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(led_gpiod);
    }

    /* Register sysfs file */
    ret = device_create_file(dev, &dev_attr_control);
    if (ret) {
        dev_err(dev, "Failed to create sysfs entry\n");
        gpiod_put(led_gpiod);
        return ret;
    }

    dev_info(dev, "GPIO LED driver bound successfully\n");
    return 0;
}

static int gpioled_remove(struct platform_device *pdev)
{
    /* Remove sysfs entry */
    device_remove_file(&pdev->dev, &dev_attr_control);

    /* Turn off LED and release GPIO */
    gpiod_set_value(led_gpiod, 0);
    gpiod_put(led_gpiod);

    dev_info(&pdev->dev, "GPIO LED driver removed\n");
    return 0;
}

static const struct of_device_id gpioled_of_ids[] = {
    { .compatible = "myvendor,gpioled" },
    { }
};
MODULE_DEVICE_TABLE(of, gpioled_of_ids);

static struct platform_driver gpioled_driver = {
    .driver = {
        .name = "gpioled",
        .of_match_table = gpioled_of_ids,
    },
    .probe = gpioled_probe,
    .remove = gpioled_remove,
};

module_platform_driver(gpioled_driver);

