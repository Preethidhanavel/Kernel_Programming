#include <linux/init.h>             // Module init/exit macros
#include <linux/module.h>           // MODULE_LICENSE, MODULE_AUTHOR, MODULE_DESCRIPTION
#include <linux/platform_device.h>  // Platform driver structures
#include <linux/of.h>               // Device Tree helper functions
#include <linux/of_device.h>        // Device Tree match table
#include <linux/gpio/consumer.h>    // GPIO descriptor API
#include <linux/interrupt.h>        // IRQ handling functions
#include <linux/irq.h>              // IRQ numbers and flags
#include <linux/irqreturn.h>        // IRQ_HANDLED / IRQ_NONE macros

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button with interrupt and DT-configurable trigger");

// GPIO descriptor for the button
static struct gpio_desc *btn_gpiod;

// IRQ number associated with button GPIO
static int irq_num;

/* ---------- IRQ HANDLER ----------
 * Called when a button interrupt occurs
 * Prints message to kernel log
 */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    pr_info("[gpiobtn] Interrupt received! Button event.\n");
    return IRQ_HANDLED;  // IRQ handled successfully
}

/* ---------- PROBE FUNCTION ----------
 * Called when platform device matches driver
 * Requests GPIO, converts to IRQ, and sets trigger based on DT
 */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *trigger_str;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING; /* default trigger */

    // Get GPIO from Device Tree
    btn_gpiod = gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(btn_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(btn_gpiod);
    }

    // Convert GPIO to IRQ number
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0) {
        dev_err(dev, "Failed to get IRQ from GPIO\n");
        gpiod_put(btn_gpiod);
        return irq_num;
    }

    // Read "irq-trigger" property from DT (optional)
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
    } else {
        dev_info(dev, "irq-trigger not specified, defaulting to falling\n");
    }

    // Request IRQ with chosen trigger flags
    if (request_irq(irq_num, btn_irq_handler, irq_flags,
                    "gpiobtn_irq", NULL)) {
        dev_err(dev, "Failed to request IRQ\n");
        gpiod_put(btn_gpiod);
        return -EBUSY;
    }

    dev_info(dev, "Button driver initialized, IRQ %d, trigger=%s\n",
             irq_num, trigger_str ? trigger_str : "falling");
    return 0;
}

/* ---------- REMOVE FUNCTION ----------
 * Called when driver is removed
 * Frees IRQ and GPIO
 */
static int btn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);   // Free IRQ
    gpiod_put(btn_gpiod);      // Release GPIO
    pr_info("[gpiobtn] Driver removed\n");
    return 0;
}

/* ---------- DEVICE TREE MATCH TABLE ----------
 * Matches compatible strings from DT with driver
 */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* ---------- PLATFORM DRIVER ----------
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
