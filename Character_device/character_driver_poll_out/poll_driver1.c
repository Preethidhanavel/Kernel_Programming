#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/mutex.h>

#define DEVICE_NAME "polldev"   // Device name in /dev
#define CLASS_NAME  "pollclass" // Class name in /sys/class
#define BUF_LEN 256             // Buffer length

// Device variables
static dev_t dev_num;                  // Device number (major+minor)
static struct cdev poll_cdev;          // Char device structure
static struct class *poll_class;       // Device class
static struct device *poll_device;     // Device file

// Buffer and state flags
static char buffer[BUF_LEN];           
static int data_available = 0;         // Flag if data is available to read
static int can_write = 1;              // Initially device is writable

// Wait queue and mutex for synchronization
static DECLARE_WAIT_QUEUE_HEAD(wq);
static DEFINE_MUTEX(lock);

// Called when device file is opened
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] opened\n", DEVICE_NAME);
    return 0;
}

// Called when device file is closed
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] closed\n", DEVICE_NAME);
    return 0;
}

// Read operation
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    // Wait until data is available
    wait_event_interruptible(wq, data_available);

    // Lock buffer
    mutex_lock(&lock);

    // Copy data from kernel buffer to user space
    ret = simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));

    // Mark buffer as read
    data_available = 0;
    can_write = 1;   // Writing is allowed again

    mutex_unlock(&lock);

    // Wake up writers waiting
    wake_up_interruptible(&wq);
    return ret;
}

// Write operation
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    // If already written and not yet read, block further writes
    if (!can_write)
        return -EAGAIN;

    // Limit length to buffer size
    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    // Lock buffer
    mutex_lock(&lock);

    // Copy data from user to kernel buffer
    if (copy_from_user(buffer, buf, len)) {
        mutex_unlock(&lock);
        return -EFAULT;
    }

    buffer[len] = '\0';   // Null-terminate string
    data_available = 1;   // Data is ready for reading
    can_write = 0;        // Block further writes until read occurs

    mutex_unlock(&lock);

    // Wake up readers waiting
    wake_up_interruptible(&wq);

    printk(KERN_INFO "[%s] wrote: %s\n", DEVICE_NAME, buffer);
    return len;
}

// Poll function to check device readiness
static __poll_t dev_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;

    // Register this process in wait queue
    poll_wait(file, &wq, wait);

    // If data is ready -> mark readable
    if (data_available)
        mask |= POLLIN | POLLRDNORM;

    // If buffer is free -> mark writable
    if (can_write)
        mask |= POLLOUT | POLLWRNORM;

    return mask;
}

// File operations mapping
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write,
    .poll    = dev_poll,
};

// Module init function
static int __init poll_driver_init(void)
{
    int ret;

    // Allocate major/minor number
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    // Register char device
    cdev_init(&poll_cdev, &fops);
    cdev_add(&poll_cdev, dev_num, 1);

    // Create device class and file in /dev
    poll_class = class_create(THIS_MODULE, CLASS_NAME);
    poll_device = device_create(poll_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] loaded\n", DEVICE_NAME);
    return 0;
}

// Module exit function
static void __exit poll_driver_exit(void)
{
    // Clean up: remove device and class
    device_destroy(poll_class, dev_num);
    class_destroy(poll_class);

    // Remove char device and free number
    cdev_del(&poll_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "[%s] unloaded\n", DEVICE_NAME);
}

// Register init/exit functions
module_init(poll_driver_init);
module_exit(poll_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Character device driver with poll support");
