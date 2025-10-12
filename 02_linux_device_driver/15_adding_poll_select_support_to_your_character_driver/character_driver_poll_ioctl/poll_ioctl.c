#include <linux/init.h>       // For module init/exit
#include <linux/module.h>     // For module macros
#include <linux/fs.h>         // For file_operations
#include <linux/uaccess.h>    // For copy_from_user, copy_to_user
#include <linux/cdev.h>       // For cdev utilities
#include <linux/device.h>     // For device_create / class_create
#include <linux/poll.h>       // For poll() support
#include <linux/wait.h>       // For wait queues
#include <linux/mutex.h>      // For mutex
#include <linux/ioctl.h>      // For ioctl macros

// --- IOCTL definitions ---
#define POLLDRV_IOC_MAGIC 'p'
#define POLLDRV_IOC_SET_READ_READY _IOW(POLLDRV_IOC_MAGIC, 1, int)

static int read_ready = 0;   // Flag controlled by ioctl()

// --- Device information ---
#define DEVICE_NAME "polldev"
#define CLASS_NAME  "pollclass"
#define BUF_LEN 256

static dev_t dev_num;             // Device number (major+minor)
static struct cdev poll_cdev;     // Char device structure
static struct class *poll_class;  // Device class
static struct device *poll_device;// Device entry in /dev

// --- Shared data ---
static char buffer[BUF_LEN];
static int data_available = 0;   // Data ready for reading?
static int can_write = 1;        // Writable? (blocked after write until read)

// --- Sync primitives ---
static DECLARE_WAIT_QUEUE_HEAD(wq);  // Wait queue for poll/select
static DEFINE_MUTEX(lock);           // Mutex for protecting buffer

// --- File operations ---

// Called when process opens /dev/polldev
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] opened\n", DEVICE_NAME);
    return 0;
}

// Called when process closes /dev/polldev
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "[%s] closed\n", DEVICE_NAME);
    return 0;
}

// Read handler
static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    int ret;

    // Block until data is available
    wait_event_interruptible(wq, data_available);

    mutex_lock(&lock);
    // Copy kernel buffer -> user buffer
    ret = simple_read_from_buffer(buf, len, off, buffer, strlen(buffer));
    data_available = 0;   // Mark data as consumed
    can_write = 1;        // Allow next write
    mutex_unlock(&lock);

    // Wake writers (since we consumed data)
    wake_up_interruptible(&wq);
    return ret;
}

// Write handler
static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    // If not writable -> return "try again" error
    if (!can_write)
        return -EAGAIN;

    if (len > BUF_LEN - 1)
        len = BUF_LEN - 1;

    mutex_lock(&lock);
    // Copy data from user -> kernel buffer
    if (copy_from_user(buffer, buf, len)) {
        mutex_unlock(&lock);
        return -EFAULT;
    }

    buffer[len] = '\0';
    data_available = 1;  // Now data is ready
    can_write = 0;       // Block further writes until read occurs
    mutex_unlock(&lock);

    // Wake readers
    wake_up_interruptible(&wq);
    printk(KERN_INFO "[%s] wrote: %s\n", DEVICE_NAME, buffer);
    return len;
}

// Poll handler: called during poll()/select()
static __poll_t dev_poll(struct file *file, poll_table *wait)
{
    __poll_t mask = 0;

    // Register this process in wait queue
    poll_wait(file, &wq, wait);

    // Data readable only if available and ioctl flag is set
    if (data_available && read_ready)
        mask |= POLLIN | POLLRDNORM;

    // Writable if can_write is true
    if (can_write)
        mask |= POLLOUT | POLLWRNORM;

    return mask;
}

// Ioctl handler
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int valu
