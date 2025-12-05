#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>

MODULE_LICENSE("GPL");


static struct gpio_desc *led_gpiod;
static bool led_state;

static int gpioled_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    led_gpiod = gpiod_get(dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod))
        return PTR_ERR(led_gpiod);

    gpiod_set_value(led_gpiod, 1);
    led_state = true;
    return 0;
}

static int gpioled_remove(struct platform_device *pdev)
{
    gpiod_set_value(led_gpiod, 0);
    gpiod_put(led_gpiod);
    return 0;
}

static int gpioled_suspend(struct device *dev)
{
    pr_info("[gpioled] Suspending...\n");
    gpiod_set_value(led_gpiod, 0);
    return 0;
}

static int gpioled_resume(struct device *dev)
{
    pr_info("[gpioled] Resuming...\n");
    if (led_state)
        gpiod_set_value(led_gpiod, 1);
    return 0;
}

static const struct dev_pm_ops gpioled_pm_ops = {
    .suspend = gpioled_suspend,
    .resume  = gpioled_resume,
};

static const struct of_device_id gpioled_of_ids[] = {
    { .compatible = "myvendor,gpioled" },
    { }
};
MODULE_DEVICE_TABLE(of, gpioled_of_ids);

static struct platform_driver gpioled_driver = {
    .driver = {
        .name = "gpioled",
        .of_match_table = gpioled_of_ids,
        .pm = &gpioled_pm_ops,
    },
    .probe = gpioled_probe,
    .remove = gpioled_remove,
};

module_platform_driver(gpioled_driver);
