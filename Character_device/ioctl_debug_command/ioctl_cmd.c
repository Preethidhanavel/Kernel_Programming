#include <linux/init.h>       // For module_init and module_exit macros
#include <linux/module.h>     // Core header for LKMs
#include <linux/fs.h>         // For file_operations structure
#include <linux/uaccess.h>    // For copy_to_user and copy_from_user
#include <linux/mutex.h>      // For mutex locking
#include <linux/ioctl.h>      // For ioctl macros

#define DEVICE_NAME "ioctldev"    // Name of the character device
#define BUFFER_SIZE 256           // Kernel buffer size
#define MAJOR_NUM 240             // Major number for registration

// IOCTL commands
#define IOCTL_RESET_BUFFER _IO(MAJOR_NUM, 0)   // Reset buffer command
#define IOCTL_GET_COUNT    _IOR(MAJOR_NUM, 1, int)  // Get write count
#define IOCTL_SET_DEBUG    _IOW(MAJOR_NUM, 2, int)  // Set debug mode

static int debug_mode = 0;           // Flag to enable/disable debug prints
static char buffer[BUFFER_SIZE];     // Kernel buffer
static int write_count = 0;          // Count of write operations
static DEFINE_MUTEX(buffer_lock);    // Mutex to protect buffer and write_count

// -------- Device open --------
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device opened\n", DEVICE_NAME);
    return 0;
}

// -------- Device release/close --------
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] Device closed\n", DEVICE_NAME);
    if (debug_mode) {
        printk(KERN_INFO "Debug mode of device is closed\n");
    }
    return 0;
}

// -------- Device write --------
static ssize_t dev_write(struct file *file, const char __user *buf,
                        size_t len, loff_t *ppos)
{
    if (len > BUFFER_SIZE - 1)      // Prevent buffer overflow
        len = BUFFER_SIZE - 1;

    if (copy_from_user(buffer, buf, len)) // Copy data from user
        return -EFAULT;

    buffer[len] = '\0';             // Null-terminate string
    write_count++;                   // Increment write count

    if (debug_mode) {                // Extra debug info
        printk(KERN_INFO "ioctldev: Debug - Received %zu bytes\n", len);
        printk(KERN_INFO "ioctldev: Debug - Buffer now: %s\n", buffer);
    }

    return len;                      // Return number of bytes written
}

// -------- Device read --------
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    return simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
}

// -------- Device ioctl --------
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;

    switch (cmd) {
        case IOCTL_RESET_BUFFER:    // Reset buffer and counter
            memset(buffer, 0, BUFFER_SIZE);
            write_count = 0;
            printk(KERN_INFO "ioctldev: Buffer reset\n");
            break;

        case IOCTL_GET_COUNT:       // Send write_count to user space
            if (copy_to_user((int __user *)arg, &write_count, sizeof(write_count)))
                return -EFAULT;
            break;

        case IOCTL_SET_DEBUG:       // Enable or disable debug mode
            if (copy_from_user(&value, (int __user *)arg, sizeof(int)))
                return -EFAULT;

            debug_mode = value ? 1 : 0;  // Sanitize input
            printk(KERN_INFO "ioctldev: Debug mode %s\n",
                   debug_mode ? "ENABLED" : "DISABLED");
            break;

        default:                    // Invalid command
            return -EINVAL;
    }

    return 0;
}

// -------- File operations --------
static struct file_operations fops = {
    .owner = THIS_MODULE,           // Prevent module from unloading
    .open = dev_open,               // open() system call
    .read = dev_read,               // read() system call
    .write = dev_write,             // write() system call
    .unlocked_ioctl = dev_ioctl,    // ioctl() system call
    .release = dev_release,         // close() system call
};

// -------- Module init --------
static int __init ioctl_driver_init(void)
{
    int ret = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops); // Register char device
    if (ret < 0) {
        printk(KERN_ALERT "Failed to register device\n");
        return ret;
    }

    mutex_init(&buffer_lock);       // Initialize mutex
    printk(KERN_INFO "[%s] registered with major %d\n", DEVICE_NAME, MAJOR_NUM);
    return 0;
}

// -------- Module exit --------
static void __exit ioctl_driver_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME); // Unregister device
    printk(KERN_INFO "[%s] unregistered\n", DEVICE_NAME);
}

// Register module init and exit
module_init(ioctl_driver_init);
module_exit(ioctl_driver_exit);

// -------- Module metadata --------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Char driver with ioctl and debug mode");
