#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/atomic.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button with interrupt, DT-configurable trigger, and press counter");

/* Global variables */
static struct gpio_desc *btn_gpiod;     // GPIO descriptor for button
static int irq_num;                     // IRQ number assigned to button
static atomic_t press_count = ATOMIC_INIT(0); // Counter for button presses
static struct proc_dir_entry *proc_entry;     // /proc entry pointer

/* -------- Interrupt handler -------- */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    int count = atomic_inc_return(&press_count); // atomically increment counter
    pr_info("[gpiobtn] Interrupt received! Button press count = %d\n", count);
    return IRQ_HANDLED;  // IRQ was handled successfully
}

/* -------- /proc/gpiobtn_info show function -------- */
static int gpiobtn_proc_show(struct seq_file *m, void *v)
{
    seq_printf(m, "driver: GPIO button\n");
    seq_printf(m, "IRQ number %d\n", irq_num);
    seq_printf(m, "press count %d\n", atomic_read(&press_count));
    return 0;
}

/* -------- /proc open function -------- */
static int gpiobtn_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, gpiobtn_proc_show, NULL);
}

/* -------- proc_ops structure -------- */
static const struct proc_ops gpiobtn_proc_ops = {
    .proc_open    = gpiobtn_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* -------- Probe function (called when device is matched) -------- */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const char *trigger_str;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING; /* default trigger */

    /* Get button GPIO from DT */
    btn_gpiod = gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(btn_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(btn_gpiod);
    }

    /* Convert GPIO to IRQ */
    irq_num = gpiod_to_irq(btn_gpiod);
    if (irq_num < 0) {
        dev_err(dev, "Failed to get IRQ from GPIO\n");
        gpiod_put(btn_gpiod);
        return irq_num;
    }

    /* Read "irq-trigger" property from DT (rising/falling/both) */
    if (!of_property_read_string(dev->of_node, "irq-trigger", &trigger_str)) {
        if (!strcmp(trigger_str, "rising"))
            irq_flags = IRQF_TRIGGER_RISING;
        else if (!strcmp(trigger_str, "falling"))
            irq_flags = IRQF_TRIGGER_FALLING;
        else if (!strcmp(trigger_str, "both"))
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
        else
            dev_warn(dev, "Unknown irq-trigger '%s', defaulting to falling\n",
                     trigger_str);
    } else {
        dev_info(dev, "irq-trigger not specified, defaulting to falling\n");
    }

    /* Request IRQ for button */
    if (request_irq(irq_num, btn_irq_handler, irq_flags,
                    "gpiobtn_irq", NULL)) {
        dev_err(dev, "Failed to request IRQ\n");
        gpiod_put(btn_gpiod);
        return -EBUSY;
    }

    /* Create /proc entry */
    proc_entry = proc_create("gpiobtn_info", 0, NULL, &gpiobtn_proc_ops);
    if (!proc_entry)
        dev_warn(dev, "failed to create /proc/gpiobtn_info\n");

    atomic_set(&press_count, 0); // reset press count

    dev_info(dev, "Button driver initialized, IRQ %d, trigger=%s\n",
             irq_num, trigger_str ? trigger_str : "falling");
    return 0;
}

/* -------- Remove function (cleanup when driver is removed) -------- */
static void btn_remove(struct platform_device *pdev)
{
    proc_remove(proc_entry);       // remove /proc entry
    free_irq(irq_num, NULL);       // free IRQ
    gpiod_put(btn_gpiod);          // release GPIO
    pr_info("[gpiobtn] Driver removed. Final press count = %d\n",
            atomic_read(&press_count));
}

/* -------- Device Tree match table -------- */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* -------- Platform driver structure -------- */
static struct platform_driver btn_driver = {
    .driver = {
        .name           = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe  = btn_probe,
    .remove = btn_remove,
};

/* -------- Register platform driver -------- */
module_platform_driver(btn_driver);
