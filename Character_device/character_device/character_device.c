#include <linux/init.h>       // Needed for module init/exit macros
#include <linux/module.h>     // Needed for all modules
#include <linux/fs.h>         // Needed for file operations structure
#include <linux/uaccess.h>    // Needed for copy_to_user/copy_from_user

#define DEVICE_NAME "mychardev"   // Name of the device
#define BUFFER_SIZE 1024          // Size of the buffer

static int major;                 // Will hold the major number assigned
static char device_buffer[BUFFER_SIZE]; // Kernel buffer to store data
static int open_count = 0;        // Counter to track how many times device is opened

// Called when the device is opened
static int dev_open(struct inode *inode, struct file *file)
{
    open_count++;
    printk(KERN_INFO "[%s] Device opened %d times\n", DEVICE_NAME, open_count);
    return 0;
}

// Called when a user reads from the device
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    // Copy data from kernel buffer to user buffer
    return simple_read_from_buffer(buf, len, offset, device_buffer, strlen(device_buffer));
}

// Called when a user writes to the device 
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    // Limit data size to BUFFER_SIZE - 1 (for null terminator)
    if (len > BUFFER_SIZE - 1)
        len = BUFFER_SIZE - 1;

    // Copy data from user space to kernel buffer
    if (copy_from_user(device_buffer, buf, len))
        return -EFAULT;

    // Null terminate the string
    device_buffer[len] = '\0';
    printk(KERN_INFO "[%s] Received: %s\n", DEVICE_NAME, device_buffer);
    return len;
}

// Called when the device is closed
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device closed\n", DEVICE_NAME);
    return 0;
}

// File operations structure - tells kernel which function to call
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .read    = dev_read,
    .write   = dev_write,
    .release = dev_release,
};

// Module initialization
static int __init char_driver_init(void)
{
    // Register character device dynamically
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major;
    }

    printk(KERN_INFO "Char driver registered with major number %d\n", major);
    return 0;
}

// Module cleanup
static void __exit char_driver_exit(void)
{
    // Unregister character device
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Char driver unregistered\n");
}

// Macros for module entry and exit points
module_init(char_driver_init);
module_exit(char_driver_exit);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Simple character device driver");
