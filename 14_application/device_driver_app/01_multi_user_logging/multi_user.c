#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>

#define DEVICE_NAME "mblog"

// IOCTL commands
#define MBLOG_CLEAR     _IO('m', 1)             // Clear log buffer
#define MBLOG_GET_SIZE  _IOR('m', 2, size_t)    // Get current log size

// Default log buffer size (can be changed using module_param while inserting module)
static int bufsize = 4096;
module_param(bufsize, int, 0644);
MODULE_PARM_DESC(bufsize, "Initial log buffer size");

// Device related variables
static dev_t dev_no;
static struct cdev mblog_cdev;
static struct class *mblog_class;

// Log buffer variables
static char *log_buffer;
static size_t log_len;

// Semaphore for write synchronization
static struct semaphore lock;   


// Called when user opens /dev/mblog
static int mblog_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "/dev/%s opened\n", DEVICE_NAME);
    return 0;
}

// Called when user closes the device
static int mblog_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "/dev/%s closed\n", DEVICE_NAME);
    return 0;
}

// Read data from buffer to userspace
static ssize_t mblog_read(struct file *file, char __user *ubuf, size_t len, loff_t *off)
{
    ssize_t ret;

    // If already at end of buffer, return 0 (EOF)
    if (*off >= log_len)
        return 0;

    // Lock before reading
    if (down_interruptible(&lock))
        return -ERESTARTSYS;

    // Adjust read length if user requests more than available
    if (len > (log_len - *off))
        len = log_len - *off;

    // Copy data to user buffer
    if (copy_to_user(ubuf, log_buffer + *off, len))
        ret = -EFAULT;
    else
        ret = len;

    up(&lock);
    return ret;
}

// Write data from user to kernel buffer
static ssize_t mblog_write(struct file *file, const char __user *ubuf, size_t len, loff_t *off)
{
    ssize_t ret = 0;

    // Lock buffer
    if (down_interruptible(&lock))
        return -ERESTARTSYS;

    // Check available space
    if (len > bufsize - log_len)
        len = bufsize - log_len;

    if (len == 0) {
        ret = -ENOSPC; // No space left
        goto out;
    }

    // Copy data from user
    if (copy_from_user(log_buffer + log_len, ubuf, len))
        ret = -EFAULT;
    else {
        log_len += len;
        ret = len;
    }

out:
    up(&lock);
    return ret;
}

// Write logs from another kernel module (exported function)
ssize_t mblog_write_kernel(const char *kbuf, size_t len) 
{ 
    ssize_t ret = 0;

    if (!kbuf)
        return -EINVAL;

    if (down_interruptible(&lock))
        return -ERESTARTSYS;

    if (len > bufsize - log_len)
        len = bufsize - log_len;

    if (len == 0) {
        ret = -ENOSPC;
        goto out;
    }

    memcpy(log_buffer + log_len, kbuf, len);
    log_len += len;
    ret = len;

out:
    up(&lock);
    return ret; 
}
EXPORT_SYMBOL(mblog_write_kernel);  // Allow other modules to use this function

// IOCTL handler
static long mblog_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    long ret = 0;

    if (down_interruptible(&lock))
        return -ERESTARTSYS;

    switch (cmd) {

    case MBLOG_CLEAR:     // Reset buffer
        log_len = 0;
        break;

    case MBLOG_GET_SIZE:  // Send current log size to user
        if (copy_to_user((size_t __user *)arg, &log_len, sizeof(log_len)))
            ret = -EFAULT;
        break;

    default:
        ret = -EINVAL;
    }

    up(&lock);
    return ret;
}

// File operations structure for driver
static const struct file_operations mblog_fops = {
    .owner = THIS_MODULE,
    .open = mblog_open,
    .release = mblog_release,
    .read = mblog_read,
    .write = mblog_write,
    .unlocked_ioctl = mblog_ioctl,
};


// Module initialization
static int __init mblog_init(void)
{
    int ret;

    // Allocate buffer memory
    log_buffer = kmalloc(bufsize, GFP_KERNEL);
    if (!log_buffer)
        return -ENOMEM;

    log_len = 0;

    // Initialize semaphore (binary semaphore -> value 1)
    sema_init(&lock, 1);

    // Allocate character device number
    ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    // Register cdev structure
    cdev_init(&mblog_cdev, &mblog_fops);
    ret = cdev_add(&mblog_cdev, dev_no, 1);
    if (ret)
        goto err_dev;

    // Create /dev/mblog entry
    mblog_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(mblog_class)) {
        ret = PTR_ERR(mblog_class);
        goto err_class;
    }

    device_create(mblog_class, NULL, dev_no, NULL, DEVICE_NAME);

    pr_info("mblog: loaded device created /dev/%s\n", DEVICE_NAME);
    return 0;

err_class:
    cdev_del(&mblog_cdev);
err_dev:
    unregister_chrdev_region(dev_no, 1);
    kfree(log_buffer);
    return ret;
}

// Cleanup when module unloaded
static void __exit mblog_exit(void)
{
    device_destroy(mblog_class, dev_no);
    class_destroy(mblog_class);
    cdev_del(&mblog_cdev);
    unregister_chrdev_region(dev_no, 1);
    kfree(log_buffer);

    pr_info("mblog: unloaded\n");
}

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Preethi");

module_init(mblog_init);
module_exit(mblog_exit);
