#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "devmod"
#define CLASS_NAME "devclass"
#define BUF_LEN 128

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Device driver using core module");

static dev_t dev_num;
static struct cdev dev_cdev;
static struct class *dev_class;
static struct device *dev_device;

extern void core_log_message(const char *msg);  // Declare function from core
extern const char *core_version(void);          // Declare version function

static char buffer[BUF_LEN];

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    if (len >= BUF_LEN) len = BUF_LEN - 1;

    if (copy_from_user(buffer, buf, len))
        return -EFAULT;

    buffer[len] = '\0';

    core_log_message(buffer);  // Call function from core module

    return len;
}

static int dev_open(struct inode *inode, struct file *file)
{
    // Print version from core driver
    printk(KERN_INFO "[device] using core version: %s\n", core_version());
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    return 0;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .write   = dev_write,
    .open    = dev_open,
    .release = dev_release,
};

static int __init device_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    cdev_init(&dev_cdev, &fops);
    cdev_add(&dev_cdev, dev_num, 1);

    dev_class = class_create(THIS_MODULE, CLASS_NAME);
    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] device loaded\n", DEVICE_NAME);
    return 0;
}

static void __exit device_exit(void)
{
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "[%s] device unloaded\n", DEVICE_NAME);
}

module_init(device_init);
module_exit(device_exit);

