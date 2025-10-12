#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>   // for copy_to_user, copy_from_user
#include <linux/mutex.h>     // for mutex locking
#include <linux/moduleparam.h> // for module parameters

#define MSG_LEN 100     // Max length of a single message
#define MAX_MSGS 10     // Max number of messages stored in buffer

// Module parameter: device name (can be set when inserting the module)
static char device_name[20] = "enhchardev";
module_param_string(device_name, device_name, sizeof(device_name), 0660);
MODULE_PARM_DESC(device_name, "Name of the device");

static int major;                        // Major number assigned to the device
static int msg_count = 0;                // Number of messages currently stored
static char msg_buffer[MAX_MSGS][MSG_LEN]; // Circular buffer to hold messages
static int read_index = 0, write_index = 0; // Indices for reading/writing
static DEFINE_MUTEX(buffer_lock);        // Mutex for thread safety
static int msg_len = 0;                  // Length of current message

// Called when device is opened
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device opened\n", device_name);
    return 0;
}

// Called when device is closed
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device closed\n", device_name);
    return 0;
}

// Write function: stores data into circular buffer
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    if (len > MSG_LEN - 1)   // Ensure message fits into buffer
        len = MSG_LEN - 1;

    mutex_lock(&buffer_lock);   // Acquire lock before modifying buffer

    if (msg_count >= MAX_MSGS) {
        // Buffer is full, drop the message
        printk(KERN_WARNING "[%s] Buffer full, dropping message\n", device_name);
        mutex_unlock(&buffer_lock);
        return -ENOMEM;
    }

    // Copy message from user space
    if (copy_from_user(msg_buffer[write_index], buf, len)) {
        mutex_unlock(&buffer_lock);
        return -EFAULT;
    }

    // Null terminate string
    msg_buffer[write_index][len] = '\0';

    // Update write index (circular buffer)
    write_index = (write_index + 1) % MAX_MSGS;
    msg_count++;

    mutex_unlock(&buffer_lock);   // Release lock
    return len;
}

// Read function: retrieves data from circular buffer
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    ssize_t ret;

    mutex_lock(&buffer_lock);   // Acquire lock before accessing buffer

    if (msg_count == 0) {
        // No data to read
        mutex_unlock(&buffer_lock);
        return 0;
    }

    // Get length of current message
    msg_len = strlen(msg_buffer[read_index]);

    // Adjust length if user requested more
    if (len > msg_len)
        len = msg_len;

    // Copy message to user space
    if (copy_to_user(buf, msg_buffer[read_index], len)) {
        mutex_unlock(&buffer_lock);
        return -EFAULT;
    }

    ret = len;

    // Update read index (circular buffer)
    read_index = (read_index + 1) % MAX_MSGS;
    msg_count--;

    *offset = 0;   // Reset file offset each time (important for repeated reads)

    mutex_unlock(&buffer_lock);   // Release lock
    return ret;
}

// File operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

// Module initialization function
static int __init char_enhanced_init(void)
{
    // Register char device dynamically 
    major = register_chrdev(0, device_name, &fops);
    if (major < 0) 
    {
        printk(KERN_ALERT "Failed to register device\n");
        return major;
    }

    // Initialize mutex
    mutex_init(&buffer_lock);

    printk(KERN_INFO "[%s] registered with major %d\n", device_name, major);
    return 0;
}

// Module cleanup function
static void __exit char_enhanced_exit(void)
{
    unregister_chrdev(major, device_name);
    printk(KERN_INFO "[%s] unregistered\n", device_name);
}

module_init(char_enhanced_init);
module_exit(char_enhanced_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Enhanced char device with buffer and thread safety");
