#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/err.h>

#define TIMEOUT_NSEC (1000000000L)
#define TIMEOUT_SEC  (4)

static struct hrtimer dev_hr_timer;
static unsigned int count = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;

enum hrtimer_restart timer_callback(struct hrtimer *timer)
{
    pr_info("Timer Callback function Called [%d]\n", count++);
    hrtimer_forward_now(timer, ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC));
    return HRTIMER_RESTART;
}

static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .read    = dev_read,
    .write   = dev_write,
    .open    = dev_open,
    .release = dev_release,
};

static int __init dev_driver_init(void)
{
    ktime_t ktime;

    if ((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&dev_cdev, &fops);

    if ((cdev_add(&dev_cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    if (IS_ERR(dev_class = class_create(THIS_MODULE, "dev_class"))) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "dev_device"))) {
        pr_err("Cannot create the Device\n");
        goto r_device;
    }

    ktime = ktime_set(TIMEOUT_SEC, TIMEOUT_NSEC);
    hrtimer_init(&dev_hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    dev_hr_timer.function = &timer_callback;
    hrtimer_start(&dev_hr_timer, ktime, HRTIMER_MODE_REL);

    pr_info("Device Driver Inserted\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

static void __exit dev_driver_exit(void)
{
    hrtimer_cancel(&dev_hr_timer);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Removed\n");
}

module_init(dev_driver_init);
module_exit(dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("High Resolution Timer Driver");

