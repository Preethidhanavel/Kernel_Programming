#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("GPIO LED + Button platform driver");

/* GPIO descriptor pointers for LED and Button */
static struct gpio_desc *led_gpiod;
static struct gpio_desc *btn_gpiod;

/* Work structure for periodic button polling */
static struct delayed_work button_work;

/* Workqueue handler: runs periodically to check button status */
static void button_poll_work(struct work_struct *work)
{
    int val = gpiod_get_value(btn_gpiod);   // Read button GPIO state

    if (val == 0) {   // Button pressed (active low assumed)
        pr_info("gpioled: Button Pressed\n");
        gpiod_set_value(led_gpiod, 1);   // Turn ON LED when pressed
    } else {
        gpiod_set_value(led_gpiod, 0);   // Turn OFF LED when released
    }

    /* Reschedule work after 200 ms for continuous polling */
    schedule_delayed_work(&button_work, msecs_to_jiffies(200));
}

/* Probe function: called when device is matched with DT node */
static int gpioled_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    /* Request LED GPIO (configured as output, default LOW = OFF) */
    led_gpiod = gpiod_get(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod)) {
        dev_err(dev, "Failed to get LED GPIO\n");
        return PTR_ERR(led_gpiod);
    }

    /* Request Button GPIO (configured as input) */
    btn_gpiod = gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(btn_gpiod)) {
        dev_err(dev, "Failed to get Button GPIO\n");
        gpiod_put(led_gpiod);   // Release LED GPIO before exit
        return PTR_ERR(btn_gpiod);
    }

    dev_info(dev, "GPIO LED+Button driver bound successfully\n");

    /* Initialize and start periodic button polling work */
    INIT_DELAYED_WORK(&button_work, button_poll_work);
    schedule_delayed_work(&button_work, msecs_to_jiffies(200));

    return 0;
}

/* Remove function: called when driver is unloaded */
static int gpioled_remove(struct platform_device *pdev)
{
    cancel_delayed_work_sync(&button_work);  // Stop scheduled work

    gpiod_set_value(led_gpiod, 0);   // Turn OFF LED before exit
    gpiod_put(led_gpiod);            // Release LED GPIO
    gpiod_put(btn_gpiod);            // Release Button GPIO

    dev_info(&pdev->dev, "GPIO LED+Button driver removed\n");
    return 0;
}

/* Device Tree match table */
static const struct of_device_id gpioled_of_ids[] = {
    { .compatible = "myvendor,gpioledbtn" },  // Match compatible string in DT
    { }
};
MODULE_DEVICE_TABLE(of, gpioled_of_ids);

/* Platform driver structure */
static struct platform_driver gpioled_driver = {
    .driver = {
        .name = "gpioledbtn",              // Internal driver name
        .of_match_table = gpioled_of_ids,  // Link to DT match table
    },
    .probe = gpioled_probe,   // Called when device is found
    .remove = gpioled_remove, // Called when driver is removed
};

/* Register platform driver */
module_platform_driver(gpioled_driver);
