#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "mydev"           // Base name for devices (/dev/mydev0, mydev1…)
#define CLASS_NAME "multiple_minor"   // Class name for sysfs
#define NUM_DEVICES 3                 // Number of devices (minors)
#define BUF_SIZE 256                  // Buffer size for each device

static dev_t dev_base;                // Holds major number + base minor
static struct class *my_class;        // Device class

// Structure to represent each device
struct mydev_t {
    int index;                        // Device index (0,1,2…)
    char buffer[BUF_SIZE];            // Storage buffer
    struct cdev cdev;                 // Char device structure
};
static struct mydev_t my_devices[NUM_DEVICES]; // Array of devices

// Called when process opens device
static int dev_open(struct inode *inode, struct file *file)
{
    // Get pointer to our device structure using container_of()
    struct mydev_t *dev = container_of(inode->i_cdev, struct mydev_t, cdev);

    // Store device pointer inside file->private_data for later use
    file->private_data = dev;

    printk(KERN_INFO "[mydev%d] opened\n", dev->index);
    return 0;
}

// Called when process closes device
static int dev_release(struct inode *inode, struct file *file)
{
    struct mydev_t *dev = file->private_data;
    printk(KERN_INFO "[mydev%d] closed\n", dev->index);
    return 0;
}

// Called when process reads from device
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    struct mydev_t *dev = file->private_data;

    // Copy data from kernel buffer to user buffer
    return simple_read_from_buffer(buf, len, off, dev->buffer, strlen(dev->buffer));
}

// Called when process writes to device
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    struct mydev_t *dev = file->private_data;

    // Ensure we don’t exceed buffer
    if (len > BUF_SIZE - 1)
        len = BUF_SIZE - 1;

    // Copy data from user buffer into kernel buffer
    if (copy_from_user(dev->buffer, buf, len))
        return -EFAULT;

    // Null terminate to make it a string
    dev->buffer[len] = '\0';

    printk(KERN_INFO "[mydev%d] wrote: %s\n", dev->index, dev->buffer);
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

// Module initialization
static int __init multi_dev_init(void)
{
    int ret, i;
    dev_t devno;

    // Allocate major number dynamically, with NUM_DEVICES minors
    ret = alloc_chrdev_region(&dev_base, 0, NUM_DEVICES, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "alloc_chrdev_region failed\n");
        return ret;
    }

    // Create class for sysfs (/sys/class/multiple_minor)
    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class)) {
        unregister_chrdev_region(dev_base, NUM_DEVICES);
        return PTR_ERR(my_class);
    }

    // Create each device
    for (i = 0; i < NUM_DEVICES; i++) {
        my_devices[i].index = i;                      // Set index
        devno = MKDEV(MAJOR(dev_base), i);            // Major + minor number

        // Initialize cdev and link it with fops
        cdev_init(&my_devices[i].cdev, &fops);
        my_devices[i].cdev.owner = THIS_MODULE;

        // Add cdev to kernel
        ret = cdev_add(&my_devices[i].cdev, devno, 1);
        if (ret < 0) {
            printk(KERN_ALERT "cdev_add failed for %d\n", i);
            continue;
        }

        // Create /dev/mydev<i>
        device_create(my_class, NULL, devno, NULL, "%s%d", DEVICE_NAME, i);
        printk(KERN_INFO "/dev/%s%d created\n", DEVICE_NAME, i);
    }

    return 0;
}

// Module cleanup
static void __exit multi_dev_exit(void)
{
    int i;
    // Destroy each device
    for (i = 0; i < NUM_DEVICES; i++) {
        device_destroy(my_class, MKDEV(MAJOR(dev_base), i));
        cdev_del(&my_devices[i].cdev);
    }

    // Destroy class and unregister numbers
    class_destroy(my_class);
    unregister_chrdev_region(dev_base, NUM_DEVICES);

    printk(KERN_INFO "multi_dev unloaded\n");
}

module_init(multi_dev_init);
module_exit(multi_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Multiple minor devices");
