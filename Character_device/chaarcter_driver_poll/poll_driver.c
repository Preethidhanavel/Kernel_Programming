#include <linux/init.h>       // For module init/exit macros
#include <linux/module.h>     // For module info
#include <linux/fs.h>         // For file_operations structure
#include <linux/uaccess.h>    // For copy_to_user/copy_from_user
#include <linux/cdev.h>       // For cdev utilities
#include <linux/device.h>     // For device_create / class_create
#include <linux/poll.h>       // For poll() support
#include <linux/wait.h>       // For wait queues
#include <linux/mutex.h>      // For mutex

#define DEVICE_NAME "polldev"   // Device name under /dev
#define CLASS_NAME  "pollclass" // Device class for udev
#define BUF_LEN 256             // Buffer size

// Device variables
static dev_t dev_num;              // Device number (major+minor)
static struct cdev poll_cdev;      // Char device structure
static struct class *poll_class;   // Device class
static struct device *poll_device; // Device structure

static char buffer[BUF_LEN];       // Kernel buffer to hold data
static int data_available = 0;     // Flag: data ready to read?

// Wait queue and lock
static DECLARE_WAIT_QUEUE_HEAD(wq); // Wait queue for blocking read/poll
static DEFINE_MUTEX(lock);          // Mutex to protect buffer access

// --- File operations ---

// Called when user opens /dev/polldev
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] opened\n", DEVICE_NAME);
    return 0;
}

// Called when user closes /dev/polldev
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] closed\n", DEVICE_NAME);
    return 0;
}

// Called when user reads from /dev/polldev
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    // Block until data_available becomes 1
    wait_event_interruptible(wq, data_available);

    // Lock buffer before reading
    mutex_lock(&lock);

    // Copy kernel buffer -> user buffer
    ret = simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));

    // Reset flag after reading
    data_available = 0;

    mutex_unlock(&lock);
    return ret;
}

// Called when user writes to /dev/polldev
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    // Limit length to buffer size - 1 (for '\0')
    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    mutex_lock(&lock);

    // Copy user buffer -> kernel buffer
    if (copy_from_user(buffer, buf, len)) {
        mutex_unlock(&lock);
        return -EFAULT; // Copy failed
    }

    buffer[len] = '\0';   // Null-terminate
    data_available = 1;   // Mark data ready

    mutex_unlock(&lock);

    // Wake up any process waiting on read/poll
    wake_up_interruptible(&wq);

    printk(KERN_INFO "[%s] wrote: %s\n", DEVICE_NAME, buffer);
    return len;
}

// Called when user program calls poll()/select() on /dev/polldev
static __poll_t dev_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;

    // Add this process to the wait queue (if it blocks later)
    poll_wait(file, &wq, wait);

    // If data is available, notify user-space that fd is readable
    if (data_available)
        mask |= POLLIN | POLLRDNORM;

    return mask;
}

// File operations structure
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write,
    .poll    = dev_poll,  // poll() support
};

// --- Module init/exit ---

// Init function (called when module is inserted)
static int __init poll_driver_init(void)
{
    int ret;

    // Allocate device number (major+minor)
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    // Initialize and add char device
    cdev_init(&poll_cdev, &fops);
    cdev_add(&poll_cdev, dev_num, 1);

    // Create device class
    poll_class = class_create(THIS_MODULE, CLASS_NAME);

    // Create device file under /dev
    poll_device = device_create(poll_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] loaded\n", DEVICE_NAME);
    return 0;
}

// Exit function (called when module is removed)
static void __exit poll_driver_exit(void)
{
    // Cleanup: remove device and class
    device_destroy(poll_class, dev_num);
    class_destroy(poll_class);

    // Remove char device
    cdev_del(&poll_cdev);

    // Free device number
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "[%s] unloaded\n", DEVICE_NAME);
}

module_init(poll_driver_init);
module_exit(poll_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Character device driver with poll support");
