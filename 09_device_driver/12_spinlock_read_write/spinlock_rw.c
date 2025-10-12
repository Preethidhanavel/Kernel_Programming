#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/err.h>

static DEFINE_RWLOCK(dev_rwlock); // Read-write spinlock for synchronization

unsigned long dev_global_variable = 0; // Shared variable
dev_t dev = 0;                         // Device number
static struct class *dev_class;        // Device class
static struct cdev dev_cdev;           // Character device structure
static struct task_struct *dev_thread1; // Thread 1 handle
static struct task_struct *dev_thread2; // Thread 2 handle

int thread_function1(void *pv); // Thread 1 prototype
int thread_function2(void *pv); // Thread 2 prototype

// Thread 1 - Writer
int thread_function1(void *pv)
{
    while(!kthread_should_stop()) {
        write_lock(&dev_rwlock);           // Acquire write lock
        dev_global_variable++;             // Modify shared variable
        write_unlock(&dev_rwlock);         // Release write lock
        msleep(1000);                      // Sleep for 1 second
    }
    return 0;
}

// Thread 2 - Reader
int thread_function2(void *pv)
{
    while(!kthread_should_stop()) {
        read_lock(&dev_rwlock);            // Acquire read lock
        pr_info("In Thread 2 : Read value %lu\n", dev_global_variable);
        read_unlock(&dev_rwlock);          // Release read lock
        msleep(1000);
    }
    return 0;
}

// Device file open
static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened\n");
    return 0;
}

// Device file release
static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed\n");
    return 0;
}

// Device file read
static ssize_t dev_read(struct file *filp,char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read function\n");
    return 0;
}

// Device file write
static ssize_t dev_write(struct file *filp,const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}

// File operations structure
static struct file_operations fops =
{
    .owner   = THIS_MODULE,
    .read    = dev_read,
    .write   = dev_write,
    .open    = dev_open,
    .release = dev_release,
};

// Module initialization
static int __init dev_driver_init(void)
{
    // Allocate major number
    if((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) < 0){
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    cdev_init(&dev_cdev, &fops); // Initialize character device

    if((cdev_add(&dev_cdev, dev, 1)) < 0){
        pr_info("Cannot add the device\n");
        goto r_class;
    }

    // Create device class
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"dev_class"))){
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    // Create device node
    if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "dev_device"))){
        pr_info("Cannot create the device\n");
        goto r_device;
    }

    // Start writer thread
    dev_thread1 = kthread_run(thread_function1, NULL, "dev_thread1");
    if(!dev_thread1){
        pr_err("Cannot create kthread1\n");
        goto r_device;
    }

    // Start reader thread
    dev_thread2 = kthread_run(thread_function2, NULL, "dev_thread2");
    if(!dev_thread2){
        pr_err("Cannot create kthread2\n");
        goto r_device;
    }

    pr_info("Device Driver Inserted\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    cdev_del(&dev_cdev);
    return -1;
}

// Module exit
static void __exit dev_driver_exit(void)
{
    kthread_stop(dev_thread1);       // Stop writer thread
    kthread_stop(dev_thread2);       // Stop reader thread
    device_destroy(dev_class, dev);  // Destroy device
    class_destroy(dev_class);        // Destroy class
    cdev_del(&dev_cdev);             // Delete character device
    unregister_chrdev_region(dev, 1); // Unregister device number
    pr_info("Device Driver Removed\n");
}

module_init(dev_driver_init);
module_exit(dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Read-Write Spinlock Example");
