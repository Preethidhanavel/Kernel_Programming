#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>

static unsigned int led_toggle = 0; // Simulated LED state
dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;

// Workqueue to simulate threaded IRQ
static struct work_struct simulated_work;

static void simulated_thread_fn(struct work_struct *work)
{
    led_toggle ^= 1; // Toggle LED
    pr_info("Simulated IRQ (Threaded Handler) : LED state = %d\n", led_toggle);
}

// File operations
static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device file opened\n");
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device file closed\n");
    return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    uint8_t state = led_toggle;
    if (copy_to_user(buf, &state, 1))
        pr_err("Failed to copy to user\n");
    return 0;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    char rec_buf[1];

    if (copy_from_user(rec_buf, buf, 1))
        pr_err("Failed to copy from user\n");

    if (rec_buf[0] == '1')
        led_toggle = 1;
    else if (rec_buf[0] == '0')
        led_toggle = 0;

    // Simulate threaded IRQ by scheduling work
    schedule_work(&simulated_work);

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
    // Allocate device number
    if (alloc_chrdev_region(&dev, 0, 1, "sim_device") < 0)
    {
        pr_err("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    // Initialize cdev
    cdev_init(&dev_cdev, &fops);
    if (cdev_add(&dev_cdev, dev, 1) < 0)
    {
        pr_err("Cannot add cdev\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    // Create device class
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "sim_class")))
    {
        pr_err("Cannot create class\n");
        cdev_del(&dev_cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    // Create device
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "sim_device")))
    {
        pr_err("Cannot create device\n");
        class_destroy(dev_class);
        cdev_del(&dev_cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    // Initialize work
    INIT_WORK(&simulated_work, simulated_thread_fn);

    pr_info("Simulated Threaded IRQ Device Driver Inserted\n");
    return 0;
}

static void __exit dev_driver_exit(void)
{
    flush_work(&simulated_work);
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Simulated Threaded IRQ Device Driver Removed\n");
}

module_init(dev_driver_init);
module_exit(dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Simulated Threaded IRQ Driver (No Hardware)");

