#include <linux/init.h>    // For module init/exit macros
#include <linux/module.h>  // Core header for LKMs
#include <linux/fs.h>      // For file_operations structure
#include <linux/uaccess.h> // For copy_to_user / copy_from_user

#define DEVICE_NAME "mychardev"  // Name of the device
#define BUFFER_SIZE 1024          // Size of internal buffer

static int major;                 // Major number assigned to device
static char device_buffer[BUFFER_SIZE]; // Kernel buffer to store data
static int open_count = 0;        // Count how many times device opened

// Function called when device is opened
static int dev_open(struct inode *inode, struct file *file)
{
    open_count++; // Increment open counter
    printk(KERN_INFO "[%s] Device opened %d times\n", DEVICE_NAME, open_count);
    return 0;
}

// Function called when device is read
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    // Safely copy data from kernel buffer to user-space
    return simple_read_from_buffer(buf, len, offset, device_buffer, strlen(device_buffer));
}

// Function called when device is written to
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    // Prevent buffer overflow
    if (len > BUFFER_SIZE - 1)
        len = BUFFER_SIZE - 1;

    // Copy data from user-space to kernel buffer
    if (copy_from_user(device_buffer, buf, len))
        return -EFAULT;

    device_buffer[len] = '\0'; // Null terminate the string
    printk(KERN_INFO "[%s] Received: %s\n", DEVICE_NAME, device_buffer);
    return len; // Return number of bytes written
}

// Function called when device is closed
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device closed\n", DEVICE_NAME);
    return 0;
}

// File operations structure linking file operations to our functions
static struct file_operations fops = {
    .owner   = THIS_MODULE,  // Prevent module from being unloaded while in use
    .open    = dev_open,     // open() system call
    .read    = dev_read,     // read() system call
    .write   = dev_write,    // write() system call
    .release = dev_release,  // close() system call
};

// Module initialization function
static int __init char_driver_init(void)
{
    // Register character device, dynamically allocate major number
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major; // Return error if registration failed
    }

    printk(KERN_INFO "Char driver registered with major number %d\n", major);
    return 0; // Success
}

// Module exit function
static void __exit char_driver_exit(void)
{
    // Unregister character device
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Char driver unregistered\n");
}

// Register module entry and exit points
module_init(char_driver_init);
module_exit(char_driver_exit);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Simple character device driver");
