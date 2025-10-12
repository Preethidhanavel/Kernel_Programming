#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button driver with LED toggle in workqueue");

// GPIO descriptors for button and LED
static struct gpio_desc *btn_gpiod;
static struct gpio_desc *led_gpiod;

// IRQ number assigned to button
static int irq_num;
// Count how many times button was pressed
static int press_count = 0;

// Work structure to defer work from interrupt
static struct work_struct btn_work;
// Store last button press timestamp for debounce
static ktime_t last_time;

/* Workqueue handler for button press */
static void btn_work_handler(struct work_struct *work)
{
    ktime_t now = ktime_get();  // get current time
    s64 diff_ms = ktime_to_ms(ktime_sub(now, last_time));

    // Simple debounce check: ignore if <100ms since last press
    if (diff_ms < 100) {
        pr_info("[gpiobtn] Bounce ignored\n");
        return;
    }

    last_time = now;  // update timestamp

    // Count the button press
    press_count++;
    pr_info("[gpiobtn] Button pressed %d times\n", press_count);

    // Toggle LED state
    static bool led_on = false;
    led_on = !led_on;
    gpiod_set_value(led_gpiod, led_on);
}

/* Interrupt handler: only schedules work */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    schedule_work(&btn_work);  // defer to workqueue
    return IRQ_HANDLED;
}

/* Probe function: called when device is matched */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    INIT_WORK(&btn_work, btn_work_handler);  // initialize work item

    // Request button GPIO as input
    btn_gpiod = gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(btn_gpiod))
        return PTR_ERR(btn_gpiod);

    // Request LED GPIO as output (initially OFF)
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

    // Request IRQ on falling edge (button press)
    if (request_irq(irq_num, btn_irq_handler, IRQF_TRIGGER_FALLING,
                    "gpiobtn_irq", NULL)) {
        gpiod_put(btn_gpiod);
        gpiod_put(led_gpiod);
        return -EBUSY;
    }

    dev_info(dev, "Driver loaded with IRQ %d\n", irq_num);
    return 0;
}

/* Remove function: cleanup on driver removal */
static void btn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);          // free IRQ line
    gpiod_put(btn_gpiod);             // release button GPIO
    gpiod_put(led_gpiod);             // release LED GPIO
    cancel_work_sync(&btn_work);      // cancel pending work
    pr_info("[gpiobtn] Driver removed\n");
    return;
}

/* Match table for device tree */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* Platform driver structure */
static struct platform_driver btn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe = btn_probe,
    .remove = btn_remove,
};

// Register platform driver
module_platform_driver(btn_driver);
