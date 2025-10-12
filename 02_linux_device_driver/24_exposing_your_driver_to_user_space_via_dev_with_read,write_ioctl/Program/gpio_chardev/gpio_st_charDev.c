#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("GPIO button + LED platform driver with char device interface");

#define DEVICE_NAME "gpiobtn"       // Device name in /dev
#define CLASS_NAME  "gpiobtnclass"  // Sysfs class name

/* IOCTL definition */
#define GPIOBTN_MAGIC 'G'
#define GPIOBTN_RESET _IO(GPIOBTN_MAGIC, 0)

/* Per-device driver data */
struct gpiobtn_drvdata {
    struct gpio_desc *btn_gpiod;   // Button GPIO descriptor
    struct gpio_desc *led_gpiod;   // LED GPIO descriptor
    int irq_num;                   // IRQ number for button
    atomic_t press_count;          // Button press counter
    bool led_state;                // LED state

    /* Char device members */
    dev_t dev_num;                 // Device number
    struct cdev cdev;              // Char device structure
    struct class *devclass;        // Sysfs class
    struct device *device;         // Device in /dev
};

/* ---------- ISR ---------- */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    struct gpiobtn_drvdata *ddata = dev_id;
    int count = atomic_inc_return(&ddata->press_count); // Increment counter atomically

    pr_info("[gpiobtn] IRQ fired, press_count = %d\n", count);
    return IRQ_HANDLED;
}

/* ---------- Char Device File Ops ---------- */
static int gpiobtn_open(struct inode *inode, struct file *file)
{
    // Get driver private data from inode
    struct gpiobtn_drvdata *ddata =
        container_of(inode->i_cdev, struct gpiobtn_drvdata, cdev);
    file->private_data = ddata;
    pr_info("[gpiobtn] Device opened\n");
    return 0;
}

static int gpiobtn_release(struct inode *inode, struct file *file)
{
    pr_info("[gpiobtn] Device closed\n");
    return 0;
}

static ssize_t gpiobtn_read(struct file *file, char __user *buf,
                            size_t len, loff_t *off)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    char msg[64];
    int msg_len;

    // Format press count into string
    msg_len = snprintf(msg, sizeof(msg), "Press Count: %d\n",
                       atomic_read(&ddata->press_count));

    if (*off >= msg_len) // End of file condition
        return 0;

    // Copy data to user space
    if (copy_to_user(buf, msg, msg_len))
        return -EFAULT;

    *off += msg_len;
    return msg_len;
}

static ssize_t gpiobtn_write(struct file *file, const char __user *buf,
                             size_t len, loff_t *off)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    char input[8];

    // Limit input size
    if (len > sizeof(input) - 1)
        len = sizeof(input) - 1;

    if (copy_from_user(input, buf, len))
        return -EFAULT;

    input[len] = '\0';

    // If user writes "1", toggle LED
    if (input[0] == '1' && ddata->led_gpiod) {
        ddata->led_state = !ddata->led_state;
        gpiod_set_value(ddata->led_gpiod, ddata->led_state);
        pr_info("[gpiobtn] LED toggled via write(): %s\n",
                ddata->led_state ? "ON" : "OFF");
    }

    return len;
}

static long gpiobtn_ioctl(struct file *file, unsigned int cmd,
                          unsigned long arg)
{
    struct gpiobtn_drvdata *ddata = file->private_data;

    switch (cmd) {
    case GPIOBTN_RESET:
        // Reset button counter
        atomic_set(&ddata->press_count, 0);
        pr_info("[gpiobtn] Counter reset via ioctl()\n");
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

/* File operations */
static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = gpiobtn_open,
    .release        = gpiobtn_release,
    .read           = gpiobtn_read,
    .write          = gpiobtn_write,
    .unlocked_ioctl = gpiobtn_ioctl,
};

/* ---------- Probe ---------- */
static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpiobtn_drvdata *ddata;
    const char *trigger_str;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING;
    int ret;

    // Allocate driver data
    ddata = devm_kzalloc(dev, sizeof(*ddata), GFP_KERNEL);
    if (!ddata)
        return -ENOMEM;

    /* Get GPIOs from device tree */
    ddata->btn_gpiod = gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(ddata->btn_gpiod)) {
        dev_err(dev, "Failed to get button GPIO\n");
        return PTR_ERR(ddata->btn_gpiod);
    }

    ddata->led_gpiod = gpiod_get_optional(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(ddata->led_gpiod)) {
        dev_err(dev, "Failed to get LED GPIO\n");
        return PTR_ERR(ddata->led_gpiod);
    }

    /* Get IRQ for button */
    ddata->irq_num = gpiod_to_irq(ddata->btn_gpiod);
    if (ddata->irq_num < 0) {
        dev_err(dev, "Failed to get IRQ\n");
        return ddata->irq_num;
    }

    /* Parse trigger type from DT property */
    if (!of_property_read_string(dev->of_node, "irq-trigger", &trigger_str)) {
        if (!strcmp(trigger_str, "rising"))
            irq_flags = IRQF_TRIGGER_RISING;
        else if (!strcmp(trigger_str, "falling"))
            irq_flags = IRQF_TRIGGER_FALLING;
        else if (!strcmp(trigger_str, "both"))
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
    }

    /* Request IRQ */
    ret = request_irq(ddata->irq_num, btn_irq_handler, irq_flags,
                      dev_name(dev), ddata);
    if (ret) {
        dev_err(dev, "Failed to request IRQ\n");
        return ret;
    }

    // Initialize press counter and LED state
    atomic_set(&ddata->press_count, 0);
    ddata->led_state = false;

    /* Char device setup */
    ret = alloc_chrdev_region(&ddata->dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        goto err_irq;

    cdev_init(&ddata->cdev, &fops);
    ret = cdev_add(&ddata->cdev, ddata->dev_num, 1);
    if (ret)
        goto err_chrdev;

    ddata->devclass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ddata->devclass)) {
        ret = PTR_ERR(ddata->devclass);
        goto err_cdev;
    }

    ddata->device = device_create(ddata->devclass, NULL, ddata->dev_num,
                                  NULL, DEVICE_NAME);
    if (IS_ERR(ddata->device)) {
        ret = PTR_ERR(ddata->device);
        goto err_class;
    }

    // Save driver data in platform device
    platform_set_drvdata(pdev, ddata);

    dev_info(dev, "gpiobtn char device registered at /dev/%s\n",
             DEVICE_NAME);
    return 0;

err_class:
    class_destroy(ddata->devclass);
err_cdev:
    cdev_del(&ddata->cdev);
err_chrdev:
    unregister_chrdev_region(ddata->dev_num, 1);
err_irq:
    free_irq(ddata->irq_num, ddata);
    return ret;
}

/* ---------- Remove ---------- */
static int btn_remove(struct platform_device *pdev)
{
    struct gpiobtn_drvdata *ddata = platform_get_drvdata(pdev);

    // Cleanup char device
    device_destroy(ddata->devclass, ddata->dev_num);
    class_destroy(ddata->devclass);
    cdev_del(&ddata->cdev);
    unregister_chrdev_region(ddata->dev_num, 1);

    // Free IRQ and GPIOs
    free_irq(ddata->irq_num, ddata);
    if (ddata->led_gpiod)
        gpiod_put(ddata->led_gpiod);
    gpiod_put(ddata->btn_gpiod);

    pr_info("[gpiobtn] Driver removed. Final count = %d\n",
            atomic_read(&ddata->press_count));
    return 0;
}

/* ---------- DT Match ---------- */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

static struct platform_driver btn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe = btn_probe,
    .remove = btn_remove,
};

module_platform_driver(btn_driver);
