#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/mutex.h>

#define DEVICE_NAME "polldev"
#define CLASS_NAME  "pollclass"
#define BUF_LEN 256

static dev_t dev_num;
static struct cdev poll_cdev;
static struct class *poll_class;
static struct device *poll_device;

static char buffer[BUF_LEN];
static int data_available = 0;

static DECLARE_WAIT_QUEUE_HEAD(wq);
static DEFINE_MUTEX(lock);

static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] opened\n", DEVICE_NAME);
    return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] closed\n", DEVICE_NAME);
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    wait_event_interruptible(wq, data_available);

    mutex_lock(&lock);
    ret = simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
    data_available = 0;
    mutex_unlock(&lock);

    return ret;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    mutex_lock(&lock);
    if (copy_from_user(buffer, buf, len)) {
        mutex_unlock(&lock);
        return -EFAULT;
    }

    buffer[len] = '\0';
    data_available = 1;
    mutex_unlock(&lock);

    wake_up_interruptible(&wq);
    printk(KERN_INFO "[%s] wrote: %s\n", DEVICE_NAME, buffer);
    return len;
}

static __poll_t dev_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;

    poll_wait(file, &wq, wait);  // Register this file to the wait queue

    if (data_available)
        mask |= POLLIN | POLLRDNORM;  // Data available for reading

    return mask;
}

static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write,
    .poll    = dev_poll,
};

static int __init poll_driver_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    cdev_init(&poll_cdev, &fops);
    cdev_add(&poll_cdev, dev_num, 1);

    poll_class = class_create(THIS_MODULE, CLASS_NAME);
    poll_device = device_create(poll_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] loaded\n", DEVICE_NAME);
    return 0;
}

static void __exit poll_driver_exit(void)
{
    device_destroy(poll_class, dev_num);
    class_destroy(poll_class);
    cdev_del(&poll_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "[%s] unloaded\n", DEVICE_NAME);
}

module_init(poll_driver_init);
module_exit(poll_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("character device driver with poll");
