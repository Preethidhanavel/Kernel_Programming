#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Multi-device GPIO button + LED driver with char device, ioctl, poll/wait");

// Config
#define DEVICE_NAME "gpiobtn"
#define CLASS_NAME  "gpiobtnclass"
#define MAX_DEVICES 8    /* Maximum supported devices */

// IOCTL commands
#define GPIOBTN_MAGIC  'G'
#define GPIOBTN_RESET   _IO(GPIOBTN_MAGIC, 0)
#define GPIOBTN_LED_ON  _IO(GPIOBTN_MAGIC, 1)
#define GPIOBTN_LED_OFF _IO(GPIOBTN_MAGIC, 2)

/* Per-device data */
struct gpiobtn_drvdata {
    struct gpio_desc *btn_gpiod;   // button GPIO
    struct gpio_desc *led_gpiod;   // LED GPIO (optional)
    int irq_num;                   // IRQ number for button
    atomic_t press_count;          // button press counter
    bool led_state;                // LED ON/OFF state

    // char device members
    dev_t dev_num;
    struct cdev cdev;
    struct device *device;
    int minor;

    // wait queue + poll
    wait_queue_head_t wq;
    int data_ready;
};

/* Globals for managing multiple devices */
static dev_t base_devnum;
static struct class *gpiobtn_devclass;
static bool minor_used[MAX_DEVICES];
static DEFINE_MUTEX(minor_lock); /* protect minor allocation */

/* Allocate free minor number */
static int allocate_minor(void)
{
    int i;

    mutex_lock(&minor_lock);
    for (i = 0; i < MAX_DEVICES; i++) {
        if (!minor_used[i]) {
            minor_used[i] = true;
            mutex_unlock(&minor_lock);
            return i;
        }
    }
    mutex_unlock(&minor_lock);
    return -ENOSPC;
}

/* Free allocated minor */
static void free_minor(int minor)
{
    if (minor < 0 || minor >= MAX_DEVICES)
        return;
    mutex_lock(&minor_lock);
    minor_used[minor] = false;
    mutex_unlock(&minor_lock);
}

/* ---------- ISR ---------- */
static irqreturn_t btn_irq_handler(int irq, void *dev_id)
{
    struct gpiobtn_drvdata *ddata = dev_id;
    int count;

    // Increment press counter
    count = atomic_inc_return(&ddata->press_count);

    // Signal readers waiting on data
    ddata->data_ready = 1;
    wake_up_interruptible(&ddata->wq);

    pr_info("%s: IRQ %d: press_count=%d\n", DEVICE_NAME, irq, count);
    return IRQ_HANDLED;
}

/* ---------- Char device file ops ---------- */
static int gpiobtn_open(struct inode *inode, struct file *file)
{
    struct gpiobtn_drvdata *ddata;

    // Get device data from inode
    ddata = container_of(inode->i_cdev, struct gpiobtn_drvdata, cdev);
    if (!ddata)
        return -ENODEV;

    file->private_data = ddata;
    pr_info("%s%d: open\n", DEVICE_NAME, ddata->minor);
    return 0;
}

static int gpiobtn_release(struct inode *inode, struct file *file)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    pr_info("%s%d: release\n", DEVICE_NAME, ddata ? ddata->minor : -1);
    return 0;
}

static ssize_t gpiobtn_read(struct file *file, char __user *buf,size_t len, loff_t *off)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    char msg[64];
    int msg_len;
    int ret;

    if (!ddata)
        return -ENODEV;

    // Block until button is pressed
    ret = wait_event_interruptible(ddata->wq, ddata->data_ready);
    if (ret)
        return ret;

    // Prepare message with press count
    msg_len = snprintf(msg, sizeof(msg), "Press Count: %d\n",
                       atomic_read(&ddata->press_count));

    // Reset flag so next read blocks
    ddata->data_ready = 0;

    if (*off >= msg_len)
        return 0;

    if (len > msg_len)
        len = msg_len;

    // Copy message to user space
    if (copy_to_user(buf, msg, len))
        return -EFAULT;

    *off += len;
    return len;
}

static ssize_t gpiobtn_write(struct file *file, const char __user *buf,size_t len, loff_t *off)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    char kbuf[8];

    if (!ddata)
        return -ENODEV;

    if (!ddata->led_gpiod)
        return -ENODEV;

    if (len == 0)
        return 0;

    if (len > sizeof(kbuf) - 1)
        len = sizeof(kbuf) - 1;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';

    // If "1" is written, toggle LED
    if (kbuf[0] == '1') {
        ddata->led_state = !ddata->led_state;
        gpiod_set_value(ddata->led_gpiod, ddata->led_state);
        pr_info("%s%d: LED toggled via write(): %s\n",
                DEVICE_NAME, ddata->minor,
                ddata->led_state ? "ON" : "OFF");
    }

    return len;
}

static long gpiobtn_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct gpiobtn_drvdata *ddata = file->private_data;

    if (!ddata)
        return -ENODEV;

    switch (cmd) {
    case GPIOBTN_RESET:
        atomic_set(&ddata->press_count, 0);
        pr_info("%s%d: Counter reset via ioctl\n", DEVICE_NAME, ddata->minor);
        break;

    case GPIOBTN_LED_ON:
        if (ddata->led_gpiod) {
            ddata->led_state = true;
            gpiod_set_value(ddata->led_gpiod, 1);
            pr_info("%s%d: LED ON via ioctl\n", DEVICE_NAME, ddata->minor);
        }
        break;

    case GPIOBTN_LED_OFF:
        if (ddata->led_gpiod) {
            ddata->led_state = false;
            gpiod_set_value(ddata->led_gpiod, 0);
            pr_info("%s%d: LED OFF via ioctl\n", DEVICE_NAME, ddata->minor);
        }
        break;

    default:
        return -EINVAL;
    }

    return 0;
}

static __poll_t gpiobtn_poll(struct file *file, poll_table *wait)
{
    struct gpiobtn_drvdata *ddata = file->private_data;
    __poll_t mask = 0;

    if (!ddata)
        return 0;

    // Register wait queue for poll
    poll_wait(file, &ddata->wq, wait);

    // Data ready -> readable
    if (ddata->data_ready)
        mask |= POLLIN | POLLRDNORM;

    return mask;
}

/* File operations table */
static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .open           = gpiobtn_open,
    .release        = gpiobtn_release,
    .read           = gpiobtn_read,
    .write          = gpiobtn_write,
    .unlocked_ioctl = gpiobtn_ioctl,
    .poll           = gpiobtn_poll,
};

/* ---------- Platform probe/remove ---------- */

static int btn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpiobtn_drvdata *ddata;
    const char *trigger_str = NULL;
    unsigned long irq_flags = IRQF_TRIGGER_FALLING; /* default */
    int minor;
    int ret;

    // Allocate memory for per-device data
    ddata = devm_kzalloc(dev, sizeof(*ddata), GFP_KERNEL);
    if (!ddata)
        return -ENOMEM;

    // Get button GPIO
    ddata->btn_gpiod = devm_gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(ddata->btn_gpiod)) {
        dev_err(dev, "Failed to get button GPIO\n");
        return PTR_ERR(ddata->btn_gpiod);
    }

    // Get optional LED GPIO
    ddata->led_gpiod = devm_gpiod_get_optional(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(ddata->led_gpiod)) {
        dev_err(dev, "Failed to get LED GPIO\n");
        return PTR_ERR(ddata->led_gpiod);
    }

    // Map button GPIO to IRQ
    ddata->irq_num = gpiod_to_irq(ddata->btn_gpiod);
    if (ddata->irq_num < 0) {
        dev_err(dev, "Failed to get IRQ from GPIO\n");
        return ddata->irq_num;
    }

    // Optional: read irq-trigger from DT
    if (!of_property_read_string(dev->of_node, "irq-trigger", &trigger_str)) {
        if (!strcmp(trigger_str, "rising"))
            irq_flags = IRQF_TRIGGER_RISING;
        else if (!strcmp(trigger_str, "falling"))
            irq_flags = IRQF_TRIGGER_FALLING;
        else if (!strcmp(trigger_str, "both"))
            irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
        else
            dev_warn(dev, "Unknown irq-trigger '%s', using falling\n", trigger_str);
    }

    // Request IRQ for button
    ret = request_irq(ddata->irq_num, btn_irq_handler, irq_flags,
                      dev_name(dev), ddata);
    if (ret) {
        dev_err(dev, "Failed to request IRQ %d: %d\n", ddata->irq_num, ret);
        return ret;
    }

    // Init state
    atomic_set(&ddata->press_count, 0);
    ddata->led_state = false;
    init_waitqueue_head(&ddata->wq);
    ddata->data_ready = 0;

    // Allocate minor number
    minor = allocate_minor();
    if (minor < 0) {
        dev_err(dev, "No free minor available\n");
        ret = -ENOSPC;
        goto err_irq;
    }
    ddata->minor = minor;
    ddata->dev_num = MKDEV(MAJOR(base_devnum), minor);

    // Register cdev
    cdev_init(&ddata->cdev, &fops);
    ddata->cdev.owner = THIS_MODULE;
    ret = cdev_add(&ddata->cdev, ddata->dev_num, 1);
    if (ret) {
        dev_err(dev, "cdev_add failed: %d\n", ret);
        goto err_free_minor;
    }

    // Create /dev/gpiobtnX node
    ddata->device = device_create(gpiobtn_devclass, NULL,
                                  ddata->dev_num, NULL,
                                  DEVICE_NAME "%d", ddata->minor);
    if (IS_ERR(ddata->device)) {
        ret = PTR_ERR(ddata->device);
        dev_err(dev, "device_create failed: %d\n", ret);
        goto err_cdev_del;
    }

    platform_set_drvdata(pdev, ddata);

    dev_info(dev, "%s%d registered (IRQ=%d)\n",
             DEVICE_NAME, ddata->minor, ddata->irq_num);
    return 0;

err_cdev_del:
    cdev_del(&ddata->cdev);
err_free_minor:
    free_minor(minor);
err_irq:
    free_irq(ddata->irq_num, ddata);
    return ret;
}

static void btn_remove(struct platform_device *pdev)
{
    struct gpiobtn_drvdata *ddata = platform_get_drvdata(pdev);

    if (!ddata)
        return ;

    dev_info(&pdev->dev, "Removing %s%d\n", DEVICE_NAME, ddata->minor);

    // Cleanup device + cdev
    device_destroy(gpiobtn_devclass, ddata->dev_num);
    cdev_del(&ddata->cdev);
    free_minor(ddata->minor);

    // Free IRQ
    free_irq(ddata->irq_num, ddata);

    pr_info("%s%d removed. final press_count=%d\n",
            DEVICE_NAME, ddata->minor,
            atomic_read(&ddata->press_count));
}

/* Device tree match table */
static const struct of_device_id btn_of_match[] = {
    { .compatible = "myvendor,gpiobtn" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, btn_of_match);

/* Platform driver */
static struct platform_driver btn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = btn_of_match,
    },
    .probe  = btn_probe,
    .remove = btn_remove,
};

/* ---------- module init/exit ---------- */

static int __init gpiobtn_init(void)
{
    int ret;

    // Reserve char device numbers
    ret = alloc_chrdev_region(&base_devnum, 0, MAX_DEVICES, DEVICE_NAME);
    if (ret) {
        pr_err("%s: alloc_chrdev_region failed: %d\n", DEVICE_NAME, ret);
        return ret;
    }

    // Create device class
    gpiobtn_devclass = class_create(CLASS_NAME);
    if (IS_ERR(gpiobtn_devclass)) {
        unregister_chrdev_region(base_devnum, MAX_DEVICES);
        pr_err("%s: class_create failed\n", DEVICE_NAME);
        return PTR_ERR(gpiobtn_devclass);
    }

    pr_info("%s: base major=%d\n", DEVICE_NAME, MAJOR(base_devnum));
    return platform_driver_register(&btn_driver);
}

static void __exit gpiobtn_exit(void)
{
    platform_driver_unregister(&btn_driver);
    class_destroy(gpiobtn_devclass);
    unregister_chrdev_region(base_devnum, MAX_DEVICES);
    pr_info("%s: unloaded\n", DEVICE_NAME);
}

module_init(gpiobtn_init);
module_exit(gpiobtn_exit);
