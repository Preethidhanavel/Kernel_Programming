#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>

#define DEVICE_NAME "waitqdev"   // Device name
#define CLASS_NAME "waitclass"   // Class name
#define BUF_LEN 256              // Buffer size

// Device variables
static dev_t dev_num;                  // Device number (major + minor)
static struct class *waitq_class;      // Device class
static struct device *waitq_device;    // Device struct
static struct cdev waitq_cdev;         // Char device structure

// Buffer and synchronization
static char buffer[BUF_LEN];           // Storage buffer
static int data_available = 0;         // Flag to indicate new data
static DECLARE_WAIT_QUEUE_HEAD(wq);    // Wait queue
static DEFINE_MUTEX(lock);             // Mutex for critical sections

// File open
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] opened\n", DEVICE_NAME);
    return 0;
}

// File release (close)
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] closed\n", DEVICE_NAME);
    return 0;
}

// Read operation
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    // If in non-blocking mode and no data ->  return immediately with -EAGAIN
    if (file->f_flags & O_NONBLOCK) {
        if (!data_available)
            return -EAGAIN;
    } 
    else {
        // Blocking mode -> wait until data is available
        wait_event_interruptible(wq, data_available);
    }

    // Copy data from kernel buffer to user space
    mutex_lock(&lock);
    ret = simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
    data_available = 0;  // Mark data as consumed
    mutex_unlock(&lock);

    return ret;
}

// Write operation
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    int ret;

    // Limit length to buffer size - 1
    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    // Copy from user space into kernel buffer
    mutex_lock(&lock);
    ret = copy_from_user(buffer, buf, len);
    buffer[len] = '\0';
    data_available = 1;  // Mark new data as available
    mutex_unlock(&lock);

    // Wake up any readers waiting on wait queue
    wake_up_interruptible(&wq);
    printk(KERN_INFO "[%s] received: %s\n", DEVICE_NAME, buffer);
    return len;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write
};

// Module initialization
static int __init waitq_init(void)
{
    int ret;

    // Allocate a device number dynamically
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    // Initialize and add char device
    cdev_init(&waitq_cdev, &fops);
    cdev_add(&waitq_cdev, dev_num, 1);

    // Create device class and device file in /dev/
    waitq_class = class_create(THIS_MODULE, CLASS_NAME);
    waitq_device = device_create(waitq_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] driver loaded\n", DEVICE_NAME);
    return 0;
}

// Module exit (cleanup)
static void __exit waitq_exit(void)
{
    // Destroy device and class, remove cdev, unregister number
    device_destroy(waitq_class, dev_num);
    class_destroy(waitq_class);
    cdev_del(&waitq_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "[%s] unloaded\n", DEVICE_NAME);
}

module_init(waitq_init);
module_exit(waitq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Character device driver with wait queue blocking read");
