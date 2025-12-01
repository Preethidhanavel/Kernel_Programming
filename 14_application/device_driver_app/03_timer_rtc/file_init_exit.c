#define pr_fmt(fmt) KBUILD_MODNAME": "fmt"\n" 
#include<linux/init.h> /* for __init,__exit,THIS_MODULE*/ 
#include<linux/module.h> /* for module_init(), module_exit()*/ 
#include<linux/cdev.h> /* for cdev_add() ,cdev_init(), struct cdev*/ 
#include<linux/fs.h> /* for alloc_chrdev_region()*/ 
#include<linux/device.h> /* for dev_t, device_create()*/ 
#include<linux/uaccess.h> 
#include<linux/err.h> 
#include<linux/timer.h> 
#include<linux/mutex.h> 
#include <linux/kernel.h> // Required for printk() 
#include<linux/ioctl.h>
#include "time.h" // Contains custom IOCTL commands

// ---------------- GLOBALS ---------------- //

DEFINE_MUTEX(mutex);               // Protect critical sections
static struct cdev c_dev;          // Character device structure
static struct class *sys;
static struct device *dev;
static dev_t device_id;

static struct timer_list timer;    // Kernel timer
static unsigned long startjiffy;   // Starting point for timer
static int count = 1;

static int starttime = 0;          // Software clock time counter
module_param(starttime, int, S_IRUGO | S_IWUSR); 
MODULE_PARM_DESC(starttime, "Initial UNIX timestamp");

static bool lock = false;          // If true, time update is disabled

// ---------------- FILE OPERATIONS ---------------- //

// Called when device is opened
static int device_open(struct inode *inode, struct file *file)
{
    pr_info("device_open()");
    return 0;
}

// Called when device is closed
static int device_release(struct inode *inode, struct file *file)
{
    pr_info("device_release()");
    return 0;
}

// Read current time from device
static ssize_t device_read(struct file *file, char __user *buffer, size_t len, loff_t *offset)
{
    ssize_t ret = 0;

    // Prevent reading multiple times using offset logic
    if (*offset == 0 && mutex_trylock(&mutex)) {
        
        char buff[30] = {0};
        ssize_t bufflen = snprintf(buff, sizeof(buff), "%d\n", starttime);

        if (copy_to_user(buffer, buff, bufflen))
            return -EFAULT;

        mutex_unlock(&mutex);

        *offset = bufflen;
        ret = bufflen;
        pr_info("Read %zd bytes", bufflen);
    }
    return ret;
}

// Write new time value to RTC
static ssize_t device_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
    int res;

    // If locked, writing (updating time) is not allowed
    if(lock)
        return -EPERM;

    if (len > 30)
        return -ENOSPC;

    // Convert user buffer to integer
    if (kstrtoint_from_user(buffer, len, 10, &res))
        return -EFAULT;

    starttime = res;
    pr_info("Written %zd bytes", len);

    return len;
}

// IOCTL commands for locking/unlocking time update
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("IOCTL called with command: %d", cmd);

    switch (cmd)
    {
        case TIME_LOCK:
            lock = true;
            break;

        case TIME_UNLOCK:
            lock = false;
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

// Increment time every 1 second (called by timer)
static void add_time(void)
{
    if (!lock)
        starttime++;

    pr_info("Updated time: %d", starttime);
}

// File operations mapping
static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
    .owner = THIS_MODULE
};

// ---------------- TIMER CALLBACK ---------------- //

static void timer_callback(struct timer_list *tim)
{
    mod_timer(tim, startjiffy + count * msecs_to_jiffies(1000)); // Re-schedule
    add_time();
    count++;
}

// ---------------- MODULE INIT ---------------- //

static int __init tim_init(void)
{
    int err;

    // Allocate character device number
    err = alloc_chrdev_region(&device_id, 0, 1, "timer_rtc");
    if (err) {
        pr_err("Failed to allocate device number");
        goto chrdev;
    }

    pr_info("Device registered: major=%d minor=%d", MAJOR(device_id), MINOR(device_id));

    // Initialize and register cdev
    cdev_init(&c_dev, &fops);
    err = cdev_add(&c_dev, device_id, 1);

    if (err) {
        pr_err("Failed to register cdev");
        goto cdev_error;
    }

    // Create class under /sys/class/
    sys = class_create(THIS_MODULE, "timer_rtc");
    if (IS_ERR(sys)) {
        err = PTR_ERR(sys);
        pr_err("class_create failed");
        goto class_error;
    }

    // Create /dev/timer_rtc device node
    dev = device_create(sys, NULL, device_id, NULL, "timer_rtc");
    if (IS_ERR(dev)) {
        err = PTR_ERR(dev);
        pr_err("device_create failed");
        goto device_error;
    }

    // Create and start timer
    timer_setup(&timer, timer_callback, 0);
    startjiffy = jiffies;
    mod_timer(&timer, startjiffy + msecs_to_jiffies(1000));

    pr_info("Timer RTC driver loaded");
    return 0;

// -------- Error Handling Rollback -------- //
device_error:
    class_destroy(sys);
class_error:
    cdev_del(&c_dev);
cdev_error:
    unregister_chrdev_region(device_id, 1);
chrdev:
    return err;
}

// ---------------- MODULE EXIT ---------------- //

static void __exit tim_exit(void)
{
    del_timer_sync(&timer);
    device_destroy(sys, device_id);
    class_destroy(sys);
    cdev_del(&c_dev);
    unregister_chrdev_region(device_id, 1);

    pr_info("Timer RTC driver removed");
}

// ---------------- MODULE INFO ---------------- //

MODULE_AUTHOR("Preethi");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple software RTC timer");

module_init(tim_init);
module_exit(tim_exit);
