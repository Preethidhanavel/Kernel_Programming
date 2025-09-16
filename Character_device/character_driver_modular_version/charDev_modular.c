#include <linux/init.h>      // For module init/exit macros
#include <linux/module.h>    // For module macros (MODULE_LICENSE, etc.)
#include <linux/fs.h>        // For file operations structure
#include <linux/uaccess.h>   // For copy_from_user
#include <linux/cdev.h>      // For cdev utilities
#include <linux/device.h>    // For device_create / class_create

#define DEVICE_NAME "devmod"   // Name of the device (/dev/devmod)
#define CLASS_NAME  "devclass" // Name of the device class
#define BUF_LEN 128            // Buffer size

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Device driver using core module");

// Device variables
static dev_t dev_num;              // Device number (major + minor)
static struct cdev dev_cdev;       // Char device structure
static struct class *dev_class;    // Device class
static struct device *dev_device;  // Device entry

// Functions declared in another (core) module
extern void core_log_message(const char *msg);  // Function to log messages
extern const char *core_version(void);          // Function to get core version

static char buffer[BUF_LEN];  // Local buffer to hold user data

// --- File operations ---

// Write handler: called when user writes to /dev/devmod
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    // Limit length so it fits in buffer
    if (len >= BUF_LEN) len = BUF_LEN - 1;

    // Copy data from user space -> kernel buffer
    if (copy_from_user(buffer, buf, len))
        return -EFAULT;  // Return error if copy fails

    buffer[len] = '\0';  // Null-terminate string

    // Call core module function to process the message
    core_log_message(buffer);

    return len;  // Return number of bytes written
}

// Open handler: called when user opens /dev/devmod
static int dev_open(struct inode *inode, struct file *file)
{
    // Print version info from core module
    printk(KERN_INFO "[device] using core version: %s\n", core_version());
    return 0;
}

// Release handler: called when user closes /dev/devmod
static int dev_release(struct inode *inode, struct file *file)
{
    return 0;
}

// File operations structure: links system calls to our handlers
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .write   = dev_write,
    .open    = dev_open,
    .release = dev_release,
};

// --- Module init/exit ---

// Init function: runs when module is inserted
static int __init device_init(void)
{
    int ret;

    // Allocate a device number dynamically
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;

    // Initialize and add char device
    cdev_init(&dev_cdev, &fops);
    cdev_add(&dev_cdev, dev_num, 1);

    // Create device class
    dev_class = class_create(THIS_MODULE, CLASS_NAME);

    // Create device file (/dev/devmod)
    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "[%s] device loaded\n", DEVICE_NAME);
    return 0;
}

// Exit function: runs when module is removed
static void __exit device_exit(void)
{
    // Destroy device file and class
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);

    // Remove char device
    cdev_del(&dev_cdev);

    // Free device number
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "[%s] device unloaded\n", DEVICE_NAME);
}

module_init(device_init);
module_exit(device_exit);
