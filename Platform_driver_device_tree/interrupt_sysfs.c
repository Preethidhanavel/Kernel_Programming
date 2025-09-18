#include <linux/init.h>             // For module init/exit macros
#include <linux/module.h>           // For MODULE_LICENSE, author, description
#include <linux/platform_device.h>  // Platform driver/device structures
#include <linux/of.h>               // Device Tree helper functions
#include <linux/of_device.h>        // Device Tree matching
#include <linux/gpio/consumer.h>    // GPIO descriptor API
#include <linux/interrupt.h>        // IRQ handling
#include <linux/irq.h>              // IRQ numbers and flags
#include <linux/irqreturn.h>        // IRQ_HANDLED / IRQ_NONE
#include <linux/atomic.h>           // Atomic variables
#include <linux/device.h>           // Class and device creation
#include <linux/sysfs.h>            // Sysfs interface helpers

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button with interrupt, sysfs interface, and LED toggle");

// GPIO descriptors
static struct gpio_desc *btn_gpiod;
static struct gpio_desc *led_gpiod;

// IRQ number for the button
static int irq_num;

// Atomic counter for button press count
static atomic_t press_count = ATOMIC_INIT(0);

// Sysfs class and device pointers
static struct class *gpiobtn_class;
static struct device *gpiobtn_dev;

/* ---------- IRQ HANDLER ----------
 * This function runs when the button is pressed.
 * Increments press_count atomically and prints message.
 */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    int count = atomic_inc_return(&press_count);  // Increment press_count
    pr_info("[gpiobtn] Button pressed! Count = %d\n", count);
    return IRQ_HANDLED;  // IRQ handled successfully
}

/* ---------- SYSFS: press_count (RO) ----------
 * Shows the current button press count in sysfs.
 */
static ssize_t press_count_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%d\n", atomic_read(&press_count));
}
static DEVICE_ATTR_RO(press_count);

/* ---------- SYSFS: toggle_led (WO) ----------
 * Toggles the LED when writing 1 to this sysfs file.
 */
static ssize_t toggle_led_store(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf, size_t count)
{
    int val;
    static bool led_on;

    if (kstrtoint(buf, 0, &val))  // Convert string to integer
        return -EINVAL;

    if (val == 1 && led_gpiod) {
        led_on = !led_on;  // Toggle LED state
        gpiod_set_value_cansleep(led_gpiod, led_on); // Update GPIO
        pr_info("[gpiobtn] LED toggled %s\n", led_on ? "ON" : "OFF");
    }

    return count;
}
static DEVICE_ATTR_WO(toggle_led);

/* ---------- PROBE FUNCTION ----------
 * Called when the platform device matches this driver.
 * Sets up GPIOs, IRQ, and sysfs interface.
 */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *trigger_str;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING; // Default IRQ trigger
    int ret;

    // Request button GPIO as input
    btn_gpiod = gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(btn_gpiod)) {
        dev_err(dev, "Failed to get button GPIO\n");
        return PTR_ERR(btn_gpiod);
    }

    // Optional LED GPIO as output
    led_gpiod = gpiod_get_optional(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod)) {
        dev_err(dev, "Failed to get LED GPIO\n");
        ret = PTR_ERR(led_gpiod);
        goto err_btn;
    }

    // Get IRQ number from button GPIO
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0) {
        dev_err(dev, "Failed to get IRQ\n");
        ret = irq_num;
        goto err_led;
    }

    // Read irq-trigger property from device tree (optional)
    if (!of_property_read_string(dev->of_node, "irq-trigger", &trigger_str)) {
        if (!strcmp(trigger_str, "rising"))
            irq_flags = IRQF_TRIGGER_RISING;
        else if (!strcmp(trigger_str, "falling"))
            irq_flags = IRQF_TRIGGER_FALLING;
        else if (!strcmp(trigger_str, "both"))
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
        else
            dev_warn(dev, "Unknown irq-trigger '%s', defaulting to falling\n", trigger_str);
    }

    // Request IRQ line
    ret = request_irq(irq_num, btn_irq_handler, irq_flags,
                      "gpiobtn_irq", NULL);
    if (ret) {
        dev_err(dev, "Failed to request IRQ\n");
        goto err_led;
    }

    atomic_set(&press_count, 0);  // Initialize counter

    // Create class and device for sysfs
    gpiobtn_class = class_create("gpiobtn");
    if (IS_ERR(gpiobtn_class)) {
        ret = PTR_ERR(gpiobtn_class);
        goto err_irq;
    }

    gpiobtn_dev = device_create(gpiobtn_class, NULL, 0, NULL, "gpiobtn0");
    if (IS_ERR(gpiobtn_dev)) {
        ret = PTR_ERR(gpiobtn_dev);
        goto err_class;
    }

    // Create sysfs attributes
    if (device_create_file(gpiobtn_dev, &dev_attr_press_count) ||
        device_create_file(gpiobtn_dev, &dev_attr_toggle_led)) {
        dev_err(dev, "Failed to create sysfs attributes\n");
        ret = -ENOMEM;
        goto err_dev;
    }

    dev_info(dev, "gpiobtn driver initialized (IRQ %d)\n", irq_num);
    return 0;

err_dev:
    device_destroy(gpiobtn_class, 0);
err_class:
    class_destroy(gpiobtn_class);
err_irq:
    free_irq(irq_num, NULL);
err_led:
    if (led_gpiod)
        gpiod_put(led_gpiod);
err_btn:
    gpiod_put(btn_gpiod);
    return ret;
}

/* ---------- REMOVE FUNCTION ----------
 * Cleans up resources when the driver is removed.
 */
static void btn_remove(struct platform_device *pdev)
{
    device_remove_file(gpiobtn_dev, &dev_attr_press_count);
    device_remove_file(gpiobtn_dev, &dev_attr_toggle_led);
    device_destroy(gpiobtn_class, 0);
    class_destroy(gpiobtn_class);

    free_irq(irq_num, NULL);
    if (led_gpiod)
        gpiod_put(led_gpiod);
    gpiod_put(btn_gpiod);

    pr_info("[gpiobtn] Driver removed. Final press_count = %d\n",
            atomic_read(&press_count));
    return;
}

/* -------- Device Tree Match Table -------- */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* -------- Platform Driver Structure -------- */
static struct platform_driver btn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe = btn_probe,
    .remove = btn_remove,
};

// Register the platform driver
module_platform_driver(btn_driver);
