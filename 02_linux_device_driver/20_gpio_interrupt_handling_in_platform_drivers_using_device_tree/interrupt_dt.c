#include <linux/init.h>             // For module init/exit macros
#include <linux/module.h>           // For MODULE_LICENSE, author, description
#include <linux/platform_device.h>  // Platform driver/device structures
#include <linux/of.h>               // Device Tree helper functions
#include <linux/gpio/consumer.h>    // GPIO descriptor API
#include <linux/interrupt.h>        // IRQ handling
#include <linux/irq.h>              // IRQ numbers and flags
#include <linux/irqreturn.h>        // IRQ_HANDLED / IRQ_NONE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button with interrupt");

// GPIO descriptor for the button
static struct gpio_desc *btn_gpiod;
// IRQ number assigned for button
static int irq_num;

/* -------- IRQ Handler --------
 * Runs in interrupt context when the button is pressed.
 * Only prints a message; minimal work in IRQ context.
 */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    pr_info("[gpiobtn] Interrupt received! Button pressed.\n");
    return IRQ_HANDLED;  // Indicate IRQ was handled
}

/* -------- Probe Function --------
 * Called when platform device matches the driver.
 * Sets up GPIO and IRQ.
 */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    // Request the button GPIO from device tree
    btn_gpiod = gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(btn_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(btn_gpiod);
    }

    // Convert GPIO to IRQ number
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0) {
        dev_err(dev, "Failed to get IRQ from GPIO\n");
        return irq_num;
    }

    // Request IRQ on falling edge (button press)
    if (request_irq(irq_num, btn_irq_handler, IRQF_TRIGGER_FALLING,
                    "gpiobtn_irq", NULL)) {
        dev_err(dev, "Failed to request IRQ\n");
        return -EBUSY;
    }

    dev_info(dev, "Button driver initialized, IRQ %d\n", irq_num);
    return 0;
}

/* -------- Remove Function --------
 * Called when driver is removed.
 * Frees IRQ and releases GPIO.
 */
static int btn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);  // Release IRQ line
    gpiod_put(btn_gpiod);     // Release GPIO
    pr_info("[gpiobtn] Driver removed\n");
    return 0;
}

/* -------- Device Tree Match Table --------
 * Matches the driver with DT node having compatible string
 */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* -------- Platform Driver Structure --------
 * Registers probe and remove callbacks with the kernel
 */
static struct platform_driver btn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe = btn_probe,
    .remove = btn_remove,
};

// Register the platform driver with the kernel
module_platform_driver(btn_driver);
