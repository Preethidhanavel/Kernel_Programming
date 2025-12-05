#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/jiffies.h>

MODULE_LICENSE("GPL");


#define DEVICE_NAME "gpiobtn"
#define CLASS_NAME  "gpiobtnclass"

static dev_t dev_num;
static struct cdev gpiobtn_cdev;
static struct class *gpiobtn_devclass;
static struct device *gpiobtn_device;
static struct gpio_desc *btn_gpiod;
static struct gpio_desc *led_gpiod;
static int irq_num;
static int press_count;
static DECLARE_WAIT_QUEUE_HEAD(wq);
static int data_ready;
static unsigned long last_press_jiffies;
static unsigned int debounce_ms = 50;

static irqreturn_t gpiobtn_irq_handler(int irq, void *dev_id)
{
    unsigned long now = jiffies;
    if (time_before(now, last_press_jiffies + msecs_to_jiffies(debounce_ms)))
        return IRQ_HANDLED;
    last_press_jiffies = now;
    press_count++;
    data_ready = 1;
    wake_up_interruptible(&wq);
    gpiod_set_value(led_gpiod, !gpiod_get_value(led_gpiod));
    return IRQ_HANDLED;
}

static int gpiobtn_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int gpiobtn_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t gpiobtn_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    wait_event_interruptible(wq, data_ready);
    data_ready = 0;
    char msg[64];
    int msg_len = snprintf(msg, sizeof(msg), "Press Count: %d\n", press_count);
    if (*off >= msg_len)
        return 0;
    if (copy_to_user(buf, msg, msg_len))
        return -EFAULT;
    *off += msg_len;
    return msg_len;
}

static __poll_t gpiobtn_poll(struct file *file, poll_table *wait)
{
    poll_wait(file, &wq, wait);
    if (data_ready)
        return POLLIN | POLLRDNORM;
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = gpiobtn_open,
    .release = gpiobtn_release,
    .read = gpiobtn_read,
    .poll = gpiobtn_poll,
};

static int gpiobtn_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;

    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&gpiobtn_cdev, &fops);
    cdev_add(&gpiobtn_cdev, dev_num, 1);
    gpiobtn_devclass = class_create(THIS_MODULE, CLASS_NAME);
    gpiobtn_device = device_create(gpiobtn_devclass, NULL, dev_num, NULL, DEVICE_NAME);

    btn_gpiod = gpiod_get_index(dev, NULL, 0, GPIOD_IN);
    if (IS_ERR(btn_gpiod))
        return PTR_ERR(btn_gpiod);

    led_gpiod = gpiod_get_index(dev, NULL, 1, GPIOD_OUT_LOW);
    if (IS_ERR(led_gpiod))
        return PTR_ERR(led_gpiod);

    irq_num = gpiod_to_irq(btn_gpiod);
    request_irq(irq_num, gpiobtn_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "gpiobtn_irq", NULL);
    return 0;
}

static int gpiobtn_remove(struct platform_device *pdev)
{
    free_irq(irq_num, NULL);
    gpiod_put(btn_gpiod);
    gpiod_put(led_gpiod);
    device_destroy(gpiobtn_devclass, dev_num);
    class_destroy(gpiobtn_devclass);
    cdev_del(&gpiobtn_cdev);
    unregister_chrdev_region(dev_num, 1);
    return 0;
}

static const struct of_device_id gpiobtn_of_ids[] = {
    { .compatible = "myvendor,gpiobtn" },
    { }
};
MODULE_DEVICE_TABLE(of, gpiobtn_of_ids);

static struct platform_driver gpiobtn_driver = {
    .driver = {
        .name = "gpiobtn",
        .of_match_table = gpiobtn_of_ids,
    },
    .probe = gpiobtn_probe,
    .remove = gpiobtn_remove,
};

module_platform_driver(gpiobtn_driver);
