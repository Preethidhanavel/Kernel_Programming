#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>   // for copy_to_user, copy_from_user
#include <linux/cdev.h>      // for cdev
#include <linux/device.h>    // for class_create, device_create

#define DEVICE_NAME "devmod"   // Device name in /dev
#define CLASS_NAME  "devclass" // Device class name
#define BUF_LEN     128        // Buffer length

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Device driver using core module");

// Device variables
static dev_t dev_num;                // Device number (major+minor)
static struct cdev dev_cdev;         // Char device structure
static struct class *dev_class;      // Device class
static struct device *dev_device;    // Device entry

// Function imported from core module
extern void core_log_message(const char *msg);

static char buffer[BUF_LEN];         // Temporary buffer (unused here)
static char device_buffer[256];      // Device storage buffer
static int data_size = 0;            // Amount of valid data in buffer

// -------- File operations --------

// Read from device
ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;

    if (*ppos >= data_size) {
        // End of file: no more data
        return 0;
    }

    // If user requests more than available, adjust count
    if (count > data_size - *ppos)
        count = data_size - *ppos;

    // Copy data from kernel buffer -> user buffer
    ret = copy_to_user(buf, device_buffer + *ppos, count);
    if (ret != 0) {
        return -EFAULT; // Copy failed
    }

    *ppos += count;   // Update file offset
    return count;     // Return number of bytes read
}

// Write to device
static ssize_t dev_write(struct file *file, const char __user *buf,
                         size_t count, loff_t *ppos)
{
    int ret;

    // Limit count if user writes more than buffer size
    if (count > sizeof(device_buffer))
        count = sizeof(device_buffer);

    // Copy data from user -> kernel buffer
    ret = copy_from_user(device_buffer, buf, count);
    if (ret != 0)
        return -EFAULT;

    data_size = count;  // Save number of bytes written
    *ppos = 0;          // Reset file offset for fresh read
    return count;       // Return number of bytes written
}

// Open device
static int dev_open(struct inode *inode, struct file *file)
{
    return 0;  // Nothing special
}

// Release (close) device
static int dev_release(struct inode *inode, struct file *file)
{
    return 0;  // Nothing special
}

// File operations structure
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .write   = dev_write,
    .open    = dev_open,
    .read    = my_read,
    .release = dev_release,
};

// -------- Module init/exit --------

// Module init function
static int __init device_init(void)
{
    int ret;

    // Allocate a device number (major + minor)
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    // Initialize and add cdev structure
    cdev_init(&dev_cdev, &fops);
    cdev_add(&dev_cdev, dev_num, 1);

    // Create class in /sys/class/
    dev_class = class_create(THIS_MODULE, CLASS_NAME);

    // Create device node in /dev/
    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] device loaded\n", DEVICE_NAME);
    return 0;
}

// Module exit function
static void __exit device_exit(void)
{
    // Destroy device node and class
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);

    // Remove cdev and free device number
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "[%s] device unloaded\n", DEVICE_NAME);
}

// Register init and exit functions
module_init(device_init);
module_exit(device_exit);
