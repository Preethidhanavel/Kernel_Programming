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

// Define a spinlock to protect the shared variable
DEFINE_SPINLOCK(dev_spinlock);
unsigned long dev_global_variable = 0;  // Shared variable incremented by threads

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;

static struct task_struct *dev_thread1; // First kernel thread
static struct task_struct *dev_thread2; // Second kernel thread

// Thread function 1
int thread_function1(void *pv)
{
    while(!kthread_should_stop()) {
        if(!spin_is_locked(&dev_spinlock)) {
            pr_info("Spinlock is not locked in Thread Function1\n");
        }
        spin_lock(&dev_spinlock); // Acquire spinlock
        if(spin_is_locked(&dev_spinlock)) {
            pr_info("Spinlock is locked in Thread Function1\n");
        }
        dev_global_variable++; // Increment shared variable
        pr_info("In thread 1 %lu\n", dev_global_variable);
        spin_unlock(&dev_spinlock); // Release spinlock
        msleep(1000); // Sleep for 1 second
    }
    return 0;
}

// Thread function 2
int thread_function2(void *pv)
{
    while(!kthread_should_stop()) {
        spin_lock(&dev_spinlock); // Acquire spinlock
        dev_global_variable++; // Increment shared variable
        pr_info("In thread 2 %lu\n", dev_global_variable);
        spin_unlock(&dev_spinlock); // Release spinlock
        msleep(1000); // Sleep for 1 second
    }
    return 0;
}

// Open function
static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

// Release function
static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

// Read function
static ssize_t dev_read(struct file *filp,char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read function\n");
    return 0;
}

// Write function
static ssize_t dev_write(struct file *filp,const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write Function\n");
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
    if((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) <0){
        pr_info("Cannot allocate major number\n");
        return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

    cdev_init(&dev_cdev,&fops); // Initialize cdev structure

    if((cdev_add(&dev_cdev,dev,1)) < 0){ // Add character device
        pr_info("Cannot add the device to the system\n");
        goto r_class;
    }

    if(IS_ERR(dev_class = class_create(THIS_MODULE,"dev_class"))){ // Create class
        pr_info("Cannot create the struct class\n");
        goto r_class;
    }

    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"dev_device"))){ // Create device
        pr_info("Cannot create the Device \n");
        goto r_device;
    }

    // Create kernel thread 1
    dev_thread1 = kthread_run(thread_function1,NULL,"dev Thread1");
    if(dev_thread1) {
        pr_err("Kthread1 Created Successfully...\n");
    } else {
        pr_err("Cannot create kthread1\n");
        goto r_device;
    }

    // Create kernel thread 2
    dev_thread2 = kthread_run(thread_function2,NULL,"dev Thread2");
    if(dev_thread2) {
        pr_err("Kthread2 Created Successfully...\n");
    } else {
        pr_err("Cannot create kthread2\n");
        goto r_device;
    }

    pr_info("Device Driver Insert...Done!!!\n");
    return 0;

r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    cdev_del(&dev_cdev);
    return -1;
}

// Module exit function
static void __exit dev_driver_exit(void)
{
    kthread_stop(dev_thread1); // Stop thread 1
    kthread_stop(dev_thread2); // Stop thread 2
    device_destroy(dev_class,dev); // Destroy device
    class_destroy(dev_class); // Destroy class
    cdev_del(&dev_cdev); // Delete cdev
    unregister_chrdev_region(dev, 1); // Free major number
    pr_info("Device Driver Remove...Done!!\n");
}

module_init(dev_driver_init);
module_exit(dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("device driver - Spinlock");
