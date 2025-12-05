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
MODULE_DESCRIPTION("Multi-device GPIO button + LED driver with char dev interface");

#define DEVICE_NAME "gpiobtn"
#define CLASS_NAME  "gpiobtnclass"
#define MAX_DEVICES 8   /* support up to 8 buttons */

/* IOCTL commands */
#define GPIOBTN_MAGIC 'G'
#define GPIOBTN_RESET   _IO(GPIOBTN_MAGIC, 0)
#define GPIOBTN_LED_ON  _IO(GPIOBTN_MAGIC, 1)
#define GPIOBTN_LED_OFF _IO(GPIOBTN_MAGIC, 2)

/* Per-device structure */
struct gpiobtn_drvdata {
    struct gpio_desc *btn_gpiod;   // GPIO for button
    struct gpio_desc *led_gpiod;   // GPIO for LED
    int irq_num;                   // IRQ number for button
    atomic_t press_count;          // count button presses
    bool led_state;                // current LED state

    dev_t dev_num;                 // device number (major+minor)
    struct cdev cdev;              // char device structure
    struct device *device;         // device entry in /dev
    int minor;                     // device minor number
};

/* Globals */
static dev_t base_devnum;               // base dev number allocated
static struct class *gpiobtn_devclass;  // device class (/sys/class)
static int device_count;                // how many devices are active

/* ---------- Interrupt Handler ---------- */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    struct gpiobtn_drvdata *ddata = dev_id;

    // increment press counter atomically
    int count = atomic_inc_return(&ddata->press_count);

    pr_info("[gpiobtn%d] IRQ fired, press_count = %d\n",
            ddata->minor, count);

    return IRQ_HANDLED;
}

/* ---------- Char Device File Operations ---------- */
static int gpiobtn_open(struct inode *inode, struct file *file)
{
    // get drvdata from inode->cdev
    struct gpiobtn_drvdata *ddata =
        container_of(inode->i_cdev, struct gpiobtn_drvdata, cdev);

    file->private_data = ddata; // save for later use
    pr_info("[gpiobtn%d] Device opened\n", ddata->minor);
    return 0;
}

static int gpiobtn_release(struct inode *inode, struct file *file)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    pr_info("[gpiobtn%d] Device closed\n", ddata->minor);
    return 0;
}

static ssize_t gpiobtn_read(struct file *file, char __user *buf,size_t len, loff_t *off)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    char msg[64];
    int msg_len;

    // prepare message with press count
    msg_len = snprintf(msg, sizeof(msg), "Press Count: %d\n",atomic_read(&ddata->press_count));

    if (*off >= msg_len)
        return 0; // EOF

    // copy message to user space
    if (copy_to_user(buf, msg, msg_len))
        return -EFAULT;

    *off += msg_len;
    return msg_len;
}

static ssize_t gpiobtn_write(struct file *file, const char __user *buf,size_t len, loff_t *off)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    char input[8];

    if (len > sizeof(input) - 1)
        len = sizeof(input) - 1;

    // copy user input
    if (copy_from_user(input, buf, len))
        return -EFAULT;

    input[len] = '\0';

    // if user writes "1" -> toggle LED
    if (input[0] == '1' && ddata->led_gpiod) {
        ddata->led_state = !ddata->led_state;
        gpiod_set_value(ddata->led_gpiod, ddata->led_state);
        pr_info("[gpiobtn%d] LED toggled via write(): %s\n",ddata->minor, ddata->led_state ? "ON" : "OFF");
    }

    return len;
}

static long gpiobtn_ioctl(struct file *file, unsigned int cmd,unsigned long arg)
{
    struct gpiobtn_drvdata *ddata = file->private_data;

    switch (cmd) {
    case GPIOBTN_RESET:
        atomic_set(&ddata->press_count, 0);
        pr_info("[gpiobtn%d] Counter reset via ioctl()\n", ddata->minor);
        break;

    case GPIOBTN_LED_ON:
        if (ddata->led_gpiod) {
            ddata->led_state = true;
            gpiod_set_value(ddata->led_gpiod, 1);
            pr_info("[gpiobtn%d] LED ON via ioctl()\n", ddata->minor);
        }
        break;

    case GPIOBTN_LED_OFF:
        if (ddata->led_gpiod) {
            ddata->led_state = false;
            gpiod_set_value(ddata->led_gpiod, 0);
            pr_info("[gpiobtn%d] LED OFF via ioctl()\n", ddata->minor);
        }
        break;

    default:
        return -EINVAL;
    }
    return 0;
}

/* file operations table */
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

    if (device_count >= MAX_DEVICES)
        return -ENODEV;

    // allocate memory for per-device data
    ddata = devm_kzalloc(dev, sizeof(*ddata), GFP_KERNEL);
    if (!ddata)
        return -ENOMEM;

    ddata->minor = device_count;

    /* Get button GPIO */
    ddata->btn_gpiod = gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(ddata->btn_gpiod))
        return PTR_ERR(ddata->btn_gpiod);

    /* Get LED GPIO (optional) */
    ddata->led_gpiod = gpiod_get_optional(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(ddata->led_gpiod))
        return PTR_ERR(ddata->led_gpiod);

    /* Convert GPIO to IRQ */
    ddata->irq_num = gpiod_to_irq(ddata->btn_gpiod);
    if (ddata->irq_num < 0)
        return ddata->irq_num;

    /* Get trigger type from DT (rising/falling/both) */
    if (!of_property_read_string(dev->of_node, "irq-trigger", &trigger_str)) {
        if (!strcmp(trigger_str, "rising"))
            irq_flags = IRQF_TRIGGER_RISING;
        else if (!strcmp(trigger_str, "falling"))
            irq_flags = IRQF_TRIGGER_FALLING;
        else if (!strcmp(trigger_str, "both"))
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
    }

    /* Request IRQ */
    ret = request_irq(ddata->irq_num, btn_irq_handler, irq_flags,dev_name(dev), ddata);
    if (ret)
        return ret;

    atomic_set(&ddata->press_count, 0);
    ddata->led_state = false;

    /* Assign device number */
    ddata->dev_num = MKDEV(MAJOR(base_devnum), ddata->minor);

    /* Register char device */
    cdev_init(&ddata->cdev, &fops);
    ret = cdev_add(&ddata->cdev, ddata->dev_num, 1);
    if (ret)
        goto err_irq;

    /* Create /dev node */
    ddata->device = device_create(gpiobtn_devclass, NULL, ddata->dev_num, NULL, DEVICE_NAME "%d", ddata->minor);
    if (IS_ERR(ddata->device)) {
        ret = PTR_ERR(ddata->device);
        goto err_cdev;
    }

    // save drvdata
    platform_set_drvdata(pdev, ddata);
    device_count++;

    dev_info(dev, "gpiobtn%d registered at /dev/gpiobtn%d (IRQ=%d)\n",ddata->minor, ddata->minor, ddata->irq_num);
    return 0;

err_cdev:
    cdev_del(&ddata->cdev);
err_irq:
    free_irq(ddata->irq_num, ddata);
    return ret;
}

/* ---------- Remove ---------- */
static void btn_remove(struct platform_device *pdev)
{
    struct gpiobtn_drvdata *ddata = platform_get_drvdata(pdev);

    device_destroy(gpiobtn_devclass, ddata->dev_num);
    cdev_del(&ddata->cdev);
    free_irq(ddata->irq_num, ddata);

    if (ddata->led_gpiod)
        gpiod_put(ddata->led_gpiod);
    gpiod_put(ddata->btn_gpiod);

    pr_info("[gpiobtn%d] Removed (final count=%d)\n",ddata->minor, atomic_read(&ddata->press_count));
}

/* ---------- Device Tree Match ---------- */
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

/* ---------- Init/Exit ---------- */
static int __init gpiobtn_init(void)
{
    int ret;

    // allocate char dev numbers for all devices
    ret = alloc_chrdev_region(&base_devnum, 0, MAX_DEVICES, DEVICE_NAME);
    if (ret < 0)
        return ret;

    // create sysfs class (/sys/class/gpiobtnclass)
    gpiobtn_devclass = class_create(CLASS_NAME);
    if (IS_ERR(gpiobtn_devclass)) {
        unregister_chrdev_region(base_devnum, MAX_DEVICES);
        return PTR_ERR(gpiobtn_devclass);
    }

    // register platform driver
    return platform_driver_register(&btn_driver);
}

static void __exit gpiobtn_exit(void)
{
    platform_driver_unregister(&btn_driver);
    class_destroy(gpiobtn_devclass);
    unregister_chrdev_region(base_devnum, MAX_DEVICES);
}

module_init(gpiobtn_init);
module_exit(gpiobtn_exit);
