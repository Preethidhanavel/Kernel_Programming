#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("GPIO button driver with LED toggle in workqueue");

// GPIO descriptors for button and LED
static struct gpio_desc *btn_gpiod;
static struct gpio_desc *led_gpiod;

// IRQ number for button
static int irq_num;
// Counter to track button presses
static int press_count = 0;

// Work structure for deferred execution
static struct work_struct btn_work;

/* -------- Workqueue Handler --------
 * Runs in process context when scheduled from IRQ handler.
 * Toggles the LED and prints button press count.
 */
static void btn_work_handler(struct work_struct *work)
{
    static bool led_on = false;   // Track LED state (OFF by default)

    press_count++;
    pr_info("[gpiobtn] Button pressed %d times (in workqueue)\n", press_count);

    led_on = !led_on;  // Toggle LED state
    gpiod_set_value(led_gpiod, led_on); // Set LED GPIO
    pr_info("[gpiobtn] LED is now %s\n", led_on ? "ON" : "OFF");
}

/* -------- Interrupt Handler --------
 * Called in interrupt context on button press.
 * Defers work to workqueue instead of doing heavy work here.
 */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    schedule_work(&btn_work);  // Schedule workqueue
    return IRQ_HANDLED;        // IRQ handled successfully
}

/* -------- Probe Function --------
 * Called when the platform device matches the driver.
 * Requests GPIOs, IRQ, and initializes workqueue.
 */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    INIT_WORK(&btn_work, btn_work_handler); // Initialize work

    // Request button GPIO as input
    btn_gpiod = gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(btn_gpiod))
        return PTR_ERR(btn_gpiod);

    // Request LED GPIO as output (default OFF)
    led_gpiod = gpiod_get(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod)) {
        gpiod_put(btn_gpiod);
        return PTR_ERR(led_gpiod);
    }

    // Convert button GPIO to IRQ number
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0) {
        gpiod_put(btn_gpiod);
        gpiod_put(led_gpiod);
        return irq_num;
    }

    // Request IRQ for falling edge (button press)
    if (request_irq(irq_num, btn_irq_handler, IRQF_TRIGGER_FALLING,
                    "gpiobtn_irq", NULL)) {
        gpiod_put(btn_gpiod);
        gpiod_put(led_gpiod);
        return -EBUSY;
    }

    dev_info(dev, "Driver loaded with IRQ %d\n", irq_num);
    return 0;
}

/* -------- Remove Function --------
 * Called when driver is removed.
 * Frees IRQ, GPIOs, and cancels any pending work.
 */
static void btn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);          // Free IRQ line
    gpiod_put(btn_gpiod);             // Release button GPIO
    gpiod_put(led_gpiod);             // Release LED GPIO
    cancel_work_sync(&btn_work);      // Cancel pending work
    pr_info("[gpiobtn] Driver removed\n");
    return;
}

/* -------- Device Tree Match Table --------
 * Matches driver to device tree node with compatible string
 */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* -------- Platform Driver Structure --------
 * Registers probe and remove functions with platform driver
 */
static struct platform_driver btn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe = btn_probe,
    .remove = btn_remove,
};

// Register the driver with the kernel
module_platform_driver(btn_driver);
