#include <linux/init.h>             // For module init/exit macros
#include <linux/module.h>           // For MODULE_LICENSE, MODULE_AUTHOR, MODULE_DESCRIPTION
#include <linux/platform_device.h>  // Platform driver structures
#include <linux/of.h>               // Device Tree helpers
#include <linux/of_device.h>        // Device Tree matching
#include <linux/gpio/consumer.h>    // GPIO descriptor API
#include <linux/interrupt.h>        // IRQ handling
#include <linux/irq.h>              // IRQ numbers and flags
#include <linux/irqreturn.h>        // IRQ_HANDLED / IRQ_NONE
#include <linux/atomic.h>           // Atomic variables

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button with interrupt, DT-configurable trigger, and press counter");

// GPIO descriptor for button
static struct gpio_desc *btn_gpiod;

// IRQ number for button
static int irq_num;

// Atomic counter for button presses
static atomic_t press_count = ATOMIC_INIT(0);

/* ---------- IRQ HANDLER ----------
 * Called when button interrupt occurs.
 * Atomically increments press_count and prints info.
 */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    int count = atomic_inc_return(&press_count);  // Increment counter atomically
    pr_info("[gpiobtn] Interrupt received! Button press count = %d\n", count);
    return IRQ_HANDLED;  // Indicate IRQ was handled
}

/* ---------- PROBE FUNCTION ----------
 * Called when platform device matches driver.
 * Sets up GPIO, IRQ, and reads "irq-trigger" from device tree.
 */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *trigger_str;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING; // Default trigger

    // Get GPIO from device tree
    btn_gpiod = gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(btn_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(btn_gpiod);
    }

    // Get IRQ number from GPIO descriptor
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0) {
        dev_err(dev, "Failed to get IRQ from GPIO\n");
        gpiod_put(btn_gpiod);
        return irq_num;
    }

    // Read "irq-trigger" property from device tree (optional)
    if (!of_property_read_string(dev->of_node, "irq-trigger", &trigger_str)) {
        if (!strcmp(trigger_str, "rising"))
            irq_flags = IRQF_TRIGGER_RISING;
        else if (!strcmp(trigger_str, "falling"))
            irq_flags = IRQF_TRIGGER_FALLING;
        else if (!strcmp(trigger_str, "both"))
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
        else
            dev_warn(dev, "Unknown irq-trigger value '%s', defaulting to falling\n",
                     trigger_str);
    } 
    else {
        dev_info(dev, "irq-trigger not specified, defaulting to falling\n");
    }

    // Request IRQ for button press
    if (request_irq(irq_num, btn_irq_handler, irq_flags,
                    "gpiobtn_irq", NULL)) {
        dev_err(dev, "Failed to request IRQ\n");
        gpiod_put(btn_gpiod);
        return -EBUSY;
    }

    atomic_set(&press_count, 0); // Initialize press counter

    dev_info(dev, "Button driver initialized, IRQ %d, trigger=%s\n",
             irq_num, trigger_str ? trigger_str : "falling");
    return 0;
}

/* ---------- REMOVE FUNCTION ----------
 * Called when driver is removed.
 * Frees IRQ and GPIO resources.
 */
static int btn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);         // Free IRQ
    gpiod_put(btn_gpiod);            // Release GPIO
    pr_info("[gpiobtn] Driver removed. Final press count = %d\n",
            atomic_read(&press_count));
    return 0;
}

/* ---------- Device Tree Match Table ----------
 * Matches driver to device tree nodes with compatible string
 */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* ---------- Platform Driver Structure ----------
 * Registers probe and remove functions with kernel
 */
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
