#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>

#define DEVICE_NAME "waitqdev"   // Device name in /dev
#define CLASS_NAME  "waitclass"  // Device class name
#define BUF_LEN 256              // Buffer size

// Device-related variables
static dev_t dev_num;                  // Device number (major+minor)
static struct class *waitq_class;      // Device class
static struct device *waitq_device;    // Device struct
static struct cdev waitq_cdev;         // Character device structure

// Shared buffer for read/write
static char buffer[BUF_LEN];           
static int data_available = 0;         // Flag: 1 = data ready, 0 = no data

// Wait queue for blocking readers
static DECLARE_WAIT_QUEUE_HEAD(wq);

// Mutex to protect buffer access
static DEFINE_MUTEX(lock);

// Counter: how many readers were blocked and later resumed
static int reader_count = 0;   

// Called when process opens the device
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] opened\n", DEVICE_NAME);
    return 0;
}

// Called when process closes the device
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] closed\n", DEVICE_NAME);
    return 0;
}

// Called when process reads from the device
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    if (file->f_flags & O_NONBLOCK) {
        // Non-blocking read -> return immediately if no data
        if (!data_available)
            return -EAGAIN;
    } 
    else {
        // Blocking read -> wait until data is available
        reader_count++;  // Count how many readers were blocked
        wait_event_interruptible(wq, data_available);
        printk(KERN_INFO "[%s] reader resumed (count=%d)\n", DEVICE_NAME, reader_count);
    }

    // Copy data safely to user buffer
    mutex_lock(&lock);
    ret = simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
    data_available = 0;  // Mark buffer empty after reading
    mutex_unlock(&lock);

    return ret;
}

// Called when process writes to the device
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    int ret;

    // Ensure data fits into buffer
    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    // Copy data from user space
    mutex_lock(&lock);
    ret = copy_from_user(buffer, buf, len);
    buffer[len] = '\0';
    data_available = 1;  // Mark data ready
    mutex_unlock(&lock);

    // Wake up any blocked readers
    wake_up_interruptible(&wq);

    printk(KERN_INFO "[%s] received: %s\n", DEVICE_NAME, buffer);
    return len;
}

// File operations structure
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write
};

// Module initialization
static int __init waitq_init(void)
{
    int ret;

    // Allocate device number
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0)
        return ret;

    // Initialize and add cdev
    cdev_init(&waitq_cdev, &fops);
    cdev_add(&waitq_cdev, dev_num, 1);

    // Create class and device in /dev
    waitq_class = class_create(THIS_MODULE, CLASS_NAME);
    waitq_device = device_create(waitq_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] driver loaded\n", DEVICE_NAME);
    return 0;
}

// Module cleanup
static void __exit waitq_exit(void)
{
    device_destroy(waitq_class, dev_num);
    class_destroy(waitq_class);
    cdev_del(&waitq_cdev);
    unregister_chrdev_region(dev_num, 1);

    // Print how many readers were blocked and resumed
    printk(KERN_INFO "[%s] unloaded (total blocked readers resumed = %d)\n",
           DEVICE_NAME, reader_count);
}

module_init(waitq_init);
module_exit(waitq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Char driver with wait queue blocking read + reader count");
