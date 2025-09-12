#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "autodev"   // Name of the device (appears in /dev/)
#define CLASS_NAME  "myclass"   // Device class name (appears in /sys/class/)
#define BUFFER_SIZE 256          // Buffer size for reading/writing

// Global variables
static dev_t dev_number;          // Device number (major + minor)
static struct cdev my_cdev;       // Character device structure
static struct class *my_class;    // Device class pointer
static struct device *my_device;  // Device pointer
static char message[BUFFER_SIZE] = "Hello from kernel space!\n"; // Kernel message buffer

// Called when user opens the device
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Opened\n", DEVICE_NAME);
    return 0;
}

// Called when user closes the device
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Closed\n", DEVICE_NAME);
    return 0;
}

// Called when user reads from the device
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    return simple_read_from_buffer(buf, len, offset, message, strlen(message));
}

// Called when user writes to the device
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    if (len > BUFFER_SIZE - 1)      // Prevent buffer overflow
        len = BUFFER_SIZE - 1;

    if (copy_from_user(message, buf, len)) // Copy data from user space
        return -EFAULT;

    message[len] = '\0';            // Null-terminate the string
    printk(KERN_INFO "[%s] Received: %s\n", DEVICE_NAME, message);
    return len;
}

// File operations structure
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .read    = dev_read,
    .write   = dev_write,
    .release = dev_release
};

// Module initialization function
static int __init autodev_init(void)
{
    int ret;

    // 1. Allocate a dynamic major number
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ALERT "alloc_chrdev_region failed\n");
        return ret;
    }

    // 2. Initialize cdev and add to kernel
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    ret = cdev_add(&my_cdev, dev_number, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_number, 1); // Cleanup
        printk(KERN_ALERT "cdev_add failed\n");
        return ret;
    }

    // 3. Create device class
    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);                     // Cleanup cdev
        unregister_chrdev_region(dev_number, 1);// Cleanup device number
        return PTR_ERR(my_class);
    }

    // 4. Create device node (/dev/autodev)
    my_device = device_create(my_class, NULL, dev_number, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);                // Cleanup class
        cdev_del(&my_cdev);                     // Cleanup cdev
        unregister_chrdev_region(dev_number, 1);// Cleanup device number
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "[%s] Device created: /dev/%s\n", DEVICE_NAME, DEVICE_NAME);
    return 0;
}

// Module cleanup function
static void __exit autodev_exit(void)
{
    device_destroy(my_class, dev_number);       // Remove device node
    class_destroy(my_class);                    // Remove class
    cdev_del(&my_cdev);                         // Delete cdev
    unregister_chrdev_region(dev_number, 1);    // Free major/minor numbers
    printk(KERN_INFO "[%s] Device removed\n", DEVICE_NAME);
}

// Register init and exit functions
module_init(autodev_init);
module_exit(autodev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Char device with automatic /dev node creation");
