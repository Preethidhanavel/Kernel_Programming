#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "mydev"            // Base name for device nodes
#define CLASS_NAME "multiple_minor"    // Device class name
#define NUM_DEVICES 3                  // Number of minor devices
#define BUF_SIZE 256                   // Per-device buffer size

// ----------------------------
// Global variables
// ----------------------------
static dev_t dev_base;                 // Base device number (major + minor 0)
static struct class *my_class;        // Device class pointer

// Device structure
struct mydev_t {
    int index;                         // Device index (minor number)
    char buffer[BUF_SIZE];             // Buffer for last message
    struct cdev cdev;                  // Character device structure
    int write_count;                   // Number of times this device has been written
};

static struct mydev_t my_devices[NUM_DEVICES]; // Array of devices

// ----------------------------
// File operations
// ----------------------------

// Open function
static int dev_open(struct inode *inode, struct file *file)
{
    // Get pointer to our device structure from inode->i_cdev
    struct mydev_t *dev = container_of(inode->i_cdev, struct mydev_t, cdev);

    file->private_data = dev;          // Save device pointer in file->private_data
    printk(KERN_INFO "[mydev%d] opened\n", dev->index);
    return 0;
}

// Release (close) function
static int dev_release(struct inode *inode, struct file *file)
{
    struct mydev_t *dev = file->private_data;
    printk(KERN_INFO "[mydev%d] closed\n", dev->index);
    return 0;
}

// Read function
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    struct mydev_t *dev = file->private_data;
    char temp[BUF_SIZE + 64];          // Extra space for count and formatting
    int msg_len;

    // Format string with write count and last message
    msg_len = snprintf(temp, sizeof(temp), "Write count = %d\nLast message: %s\n",
                       dev->write_count, dev->buffer);

    // Copy data to user-space
    return simple_read_from_buffer(buf, len, off, temp, msg_len);
}

// Write function
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    struct mydev_t *dev = file->private_data;

    if (len > BUF_SIZE - 1)            // Prevent buffer overflow
        len = BUF_SIZE - 1;

    if (copy_from_user(dev->buffer, buf, len))  // Copy from user-space
        return -EFAULT;

    dev->buffer[len] = '\0';           // Null-terminate the string
    dev->write_count++;                 // Increment per-device write count

    printk(KERN_INFO "[mydev%d] wrote: %s (write count=%d)\n",
           dev->index, dev->buffer, dev->write_count);

    return len;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

// ----------------------------
// Module initialization
// ----------------------------
static int __init multi_dev_init(void)
{
    int ret, i;
    dev_t devno;

    // Allocate dynamic major number for NUM_DEVICES
    ret = alloc_chrdev_region(&dev_base, 0, NUM_DEVICES, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "alloc_chrdev_region failed\n");
        return ret;
    }

    // Create device class for /sys/class
    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(dev_base, NUM_DEVICES);
        return PTR_ERR(my_class);
    }

    // Initialize each device
    for (i = 0; i < NUM_DEVICES; i++) {
        my_devices[i].index = i;                  // Set index
        devno = MKDEV(MAJOR(dev_base), i);       // Create device number with correct minor

        cdev_init(&my_devices[i].cdev, &fops);   // Initialize cdev
        my_devices[i].cdev.owner = THIS_MODULE;
        ret = cdev_add(&my_devices[i].cdev, devno, 1); // Add cdev to kernel
        if (ret < 0) {
            printk(KERN_ALERT "cdev_add failed for %d\n", i);
            continue;
        }

        // Create /dev/mydev0, /dev/mydev1, etc.
        device_create(my_class, NULL, devno, NULL, "%s%d", DEVICE_NAME, i);
        printk(KERN_INFO "/dev/%s%d created\n", DEVICE_NAME, i);
    }

    return 0;
}

// ----------------------------
// Module cleanup
// ----------------------------
static void __exit multi_dev_exit(void)
{
    int i;

    // Remove devices and cdevs
    for (i = 0; i < NUM_DEVICES; i++) {
        device_destroy(my_class, MKDEV(MAJOR(dev_base), i));
        cdev_del(&my_devices[i].cdev);
    }

    class_destroy(my_class);                  // Remove device class
    unregister_chrdev_region(dev_base, NUM_DEVICES); // Free major/minor numbers
    printk(KERN_INFO "multi_dev unloaded\n");
}

// Register init and exit functions
module_init(multi_dev_init);
module_exit(multi_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Multiple minor devices with per-device counters");
