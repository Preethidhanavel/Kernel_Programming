#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mychardev"   // Device name shown in /proc/devices
#define BUFFER_SIZE 1024          // Max buffer size for storing user data

static int major;                        // Major number assigned dynamically
static char device_buffer[BUFFER_SIZE];  // Kernel buffer to store data
static int open_count = 0;               // Track how many times device is opened
static size_t last_write_bytes = 0;      // Store last number of bytes written

// Called when user opens the device file
static int dev_open(struct inode *inode, struct file *file)
{
    open_count++;  // Count number of opens
    printk(KERN_INFO "[%s] Device opened %d times\n", DEVICE_NAME, open_count);
    return 0;
}

// Called when user reads from the device file
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    char temp_buf[100];  // Temporary buffer for message
    int msg_len;

    // Create a message showing how many bytes were written last time
    msg_len = snprintf(temp_buf, sizeof(temp_buf),
                       "You wrote %zu bytes last time.\n", last_write_bytes);

    // Copy message from kernel to user space
    return simple_read_from_buffer(buf, len, offset, temp_buf, msg_len);
}

// Called when user writes to the device file
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    // Limit data size to BUFFER_SIZE - 1 (for null terminator)
    if (len > BUFFER_SIZE - 1)
        len = BUFFER_SIZE - 1;

    // Copy data from user space to kernel buffer
    if (copy_from_user(device_buffer, buf, len))
        return -EFAULT;

    // Null terminate string
    device_buffer[len] = '\0';

    // Save last write byte count
    last_write_bytes = len;

    // Print info in kernel log
    printk(KERN_INFO "[%s] Received: %s\n", DEVICE_NAME, device_buffer);
    printk(KERN_INFO "[%s] Bytes written last time: %zu\n", DEVICE_NAME, last_write_bytes);

    return len;  // Return number of bytes written
}

// Called when user closes the device file
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device closed\n", DEVICE_NAME);
    return 0;
}

// File operations structure (connects system calls to our functions)
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .read    = dev_read,
    .write   = dev_write,
    .release = dev_release,
};

// Called when module is loaded
static int __init char_driver_init(void)
{
    // Register char device dynamically 
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        printk(KERN_ALERT "Failed to register character device\n");
        return major;
    }

    printk(KERN_INFO "Char driver registered with major number %d\n", major);
    return 0;
}

// Called when module is unloaded
static void __exit char_driver_exit(void)
{
    // Unregister character device
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Char driver unregistered\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Character device driver with write byte counter");
