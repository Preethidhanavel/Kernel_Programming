#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button driver with deferred work");

// GPIO descriptor for button
static struct gpio_desc *btn_gpiod;
// IRQ number assigned for button GPIO
static int irq_num;
// Counter to track button presses
static int press_count = 0;

// Work structure for deferred work
static struct work_struct btn_work;

/* -------- Workqueue Handler --------
 * This function runs in process context, not interrupt context.
 * It increments the button press counter and prints a message.
 */
static void btn_work_handler(struct work_struct *work)
{
    press_count++;
    pr_info("[gpiobtn] Button pressed %d times (in workqueue)\n", press_count);
}

/* -------- Interrupt Handler --------
 * Runs in interrupt context when the button is pressed.
 * Instead of doing heavy work here, we schedule work to the workqueue.
 */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    schedule_work(&btn_work);  // Defer processing to workqueue
    return IRQ_HANDLED;        // IRQ was successfully handled
}

/* -------- Probe Function --------
 * Called when the driver is bound to the device.
 * Sets up GPIO, IRQ, and workqueue handler.
 */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    // Initialize work structure with handler
    INIT_WORK(&btn_work, btn_work_handler);

    // Get button GPIO (from device tree)
    btn_gpiod = gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(btn_gpiod))
        return PTR_ERR(btn_gpiod);

    // Get IRQ number associated with button GPIO
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0)
        return irq_num;

    // Request IRQ for falling edge (button press)
    if (request_irq(irq_num, btn_irq_handler, IRQF_TRIGGER_FALLING,
                    "gpiobtn_irq", NULL)) {
        gpiod_put(btn_gpiod);
        return -EBUSY;
    }

    dev_info(dev, "Driver loaded with IRQ %d\n", irq_num);
    return 0;
}

/* -------- Remove Function --------
 * Cleans up resources when driver is removed.
 */
static int btn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);       // Release IRQ line
    gpiod_put(btn_gpiod);          // Release GPIO
    cancel_work_sync(&btn_work);   // Cancel pending work safely
    pr_info("[gpiobtn] Driver removed\n");
    return 0;
}

/* -------- Device Tree Match Table --------
 * Matches device node with compatible string in DT.
 */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* -------- Platform Driver Structure --------
 * Registers driver with probe and remove functions.
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
