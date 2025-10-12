#include <linux/init.h>       // For module_init and module_exit macros
#include <linux/module.h>     // Core header for LKMs
#include <linux/fs.h>         // For file_operations structure
#include <linux/uaccess.h>    // For copy_to_user and copy_from_user
#include <linux/mutex.h>      // For mutex locking
#include <linux/ioctl.h>      // For ioctl macros

#define DEVICE_NAME "ioctldev"  // Name of the device
#define BUFFER_SIZE 256          // Size of internal kernel buffer
#define MAJOR_NUM 240            // Major number for device registration

// IOCTL command definitions
#define IOCTL_RESET_BUFFER _IO(MAJOR_NUM, 0)   // Command to reset buffer
#define IOCTL_GET_COUNT    _IOR(MAJOR_NUM, 1, int) // Command to get write count

static char buffer[BUFFER_SIZE];     // Kernel buffer to store user data
static int write_count = 0;          // Counter for number of writes
static DEFINE_MUTEX(buffer_lock);    // Mutex to protect buffer and write_count

// -------- Device open --------
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device opened\n", DEVICE_NAME);
    return 0;
}

// -------- Device release (close) --------
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device closed\n", DEVICE_NAME);
    return 0;
}

// -------- Device write --------
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    if (len > BUFFER_SIZE - 1)  // Prevent buffer overflow
        len = BUFFER_SIZE - 1;

    mutex_lock(&buffer_lock);   // Lock before modifying shared buffer

    if (copy_from_user(buffer, buf, len)) {  // Copy data from user space
        mutex_unlock(&buffer_lock);
        return -EFAULT;        // Return error if copy fails
    }

    buffer[len] = '\0';         // Null-terminate the string
    write_count++;               // Increment write counter

    printk(KERN_INFO "[%s] Received: %s\n", DEVICE_NAME, buffer);

    mutex_unlock(&buffer_lock);  // Unlock after operation
    return len;                  // Return number of bytes written
}

// -------- Device read --------
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    // Copy kernel buffer to user space safely
    return simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
}

// -------- Device ioctl --------
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
        case IOCTL_RESET_BUFFER:   // Reset buffer and counter
            mutex_lock(&buffer_lock);
            memset(buffer, 0, BUFFER_SIZE);
            write_count = 0;
            mutex_unlock(&buffer_lock);
            printk(KERN_INFO "[%s] Buffer reset\n", DEVICE_NAME);
            break;

        case IOCTL_GET_COUNT:      // Send write_count to user
            if (copy_to_user((int *)arg, &write_count, sizeof(write_count)))
                return -EFAULT;   // Return error if copy fails
            printk(KERN_INFO "[%s] Sent write_count = %d\n", DEVICE_NAME, write_count);
            break;

        default:                   // Invalid command
            return -EINVAL;
    }
    return 0;
}

// -------- File operations --------
static struct file_operations fops = {
    .owner = THIS_MODULE,       // Prevent module from being unloaded
    .open = dev_open,           // open() system call
    .read = dev_read,           // read() system call
    .write = dev_write,         // write() system call
    .unlocked_ioctl = dev_ioctl,// ioctl() system call
    .release = dev_release,     // close() system call
};

// -------- Module init --------
static int __init ioctl_driver_init(void)
{
    int ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops); // Register char device
    if (ret < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return ret;
    }

    mutex_init(&buffer_lock);    // Initialize mutex
    printk(KERN_INFO "[%s] registered with major %d\n", DEVICE_NAME, MAJOR_NUM);
    return 0;
}

// -------- Module exit --------
static void __exit ioctl_driver_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME); // Unregister device
    printk(KERN_INFO "[%s] unregistered\n", DEVICE_NAME);
}

// Register init and exit functions
module_init(ioctl_driver_init);
module_exit(ioctl_driver_exit);

// -------- Module metadata --------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Char driver with ioctl");
