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
#include <linux/ktime.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Multi-device GPIO button driver with procfs, timestamp, and press counter");

/* Per-device private data */
struct gpiobtn_drvdata {
    struct gpio_desc *btn_gpiod;      // GPIO descriptor for button
    int irq_num;                      // IRQ number assigned to GPIO
    atomic_t press_count;             // Count of button presses
    struct proc_dir_entry *proc_entry; // /proc entry for this device
};

/* ---------- IRQ HANDLER ---------- */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    struct gpiobtn_drvdata *ddata = dev_id;

    // Increment button press count atomically
    int count = atomic_inc_return(&ddata->press_count);

    pr_info("[gpiobtn] IRQ %d fired! Count = %d\n", irq, count);
    return IRQ_HANDLED; // Successfully handled interrupt
}

/* ---------- PROCFS SHOW ---------- */
static int gpiobtn_proc_show(struct seq_file *m, void *v)
{
    struct gpiobtn_drvdata *ddata = m->private;

    // Print device info into /proc file
    seq_printf(m, "driver: GPIO button+LED (multi-device)\n");
    seq_printf(m, "IRQ number: %d\n", ddata->irq_num);
    seq_printf(m, "press count: %d\n", atomic_read(&ddata->press_count));
    seq_printf(m, "uptime (s): %lld\n", ktime_get_boottime_seconds());

    return 0;
}

/* Open proc file (links with seq_file interface) */
static int gpiobtn_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, gpiobtn_proc_show, pde_data(inode));
}

/* proc_ops structure to handle /proc file operations */
static const struct proc_ops gpiobtn_proc_ops = {
    .proc_open    = gpiobtn_proc_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

/* ---------- PROBE ---------- */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpiobtn_drvdata *ddata;
    const char *trigger_str;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING; /* default trigger */
    char proc_name[32];
    int ret;

    // Allocate per-device data
    ddata = devm_kzalloc(dev, sizeof(*ddata), GFP_KERNEL);
    if (!ddata)
        return -ENOMEM;

    // Get GPIO from device tree
    ddata->btn_gpiod = gpiod_get(dev, NULL, GPIOD_IN);
    if (IS_ERR(ddata->btn_gpiod)) {
        dev_err(dev, "Failed to get GPIO\n");
        return PTR_ERR(ddata->btn_gpiod);
    }

    // Convert GPIO to IRQ
    ddata->irq_num = gpiod_to_irq(ddata->btn_gpiod);
    if (ddata->irq_num < 0) {
        dev_err(dev, "Failed to get IRQ\n");
        return ddata->irq_num;
    }

    // Read "irq-trigger" property from DT
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

    // Request IRQ for button
    ret = request_irq(ddata->irq_num, btn_irq_handler, irq_flags,
                      dev_name(dev), ddata);
    if (ret) {
        dev_err(dev, "Failed to request IRQ\n");
        return ret;
    }

    // Initialize button press counter
    atomic_set(&ddata->press_count, 0);

    // Create unique /proc entry (e.g., gpiobtn0_info, gpiobtn1_info)
    snprintf(proc_name, sizeof(proc_name), "gpiobtn%d_info", pdev->id);
    ddata->proc_entry = proc_create_data(proc_name, 0444, NULL,
                                         &gpiobtn_proc_ops, ddata);
    if (!ddata->proc_entry)
        dev_warn(dev, "Failed to create /proc/%s\n", proc_name);

    // Store drvdata for later remove()
    platform_set_drvdata(pdev, ddata);

    dev_info(dev, "Button driver initialized (IRQ %d, proc: /proc/%s)\n",
             ddata->irq_num, proc_name);
    return 0;
}

/* ---------- REMOVE ---------- */
static void btn_remove(struct platform_device *pdev)
{
    struct gpiobtn_drvdata *ddata = platform_get_drvdata(pdev);

    // Remove /proc entry
    if (ddata->proc_entry)
        proc_remove(ddata->proc_entry);

    // Free IRQ and release GPIO
    free_irq(ddata->irq_num, ddata);
    gpiod_put(ddata->btn_gpiod);

    pr_info("[gpiobtn] Removed device. Final press count = %d\n",
            atomic_read(&ddata->press_count));
}

/* ---------- DT MATCH ---------- */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" }, // DT compatible string
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

/* Register platform driver */
module_platform_driver(btn_driver);
