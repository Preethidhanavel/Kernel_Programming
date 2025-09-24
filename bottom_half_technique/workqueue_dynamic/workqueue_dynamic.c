#include <linux/kernel.h>      // For printk()
#include <linux/init.h>        // For module_init and module_exit
#include <linux/module.h>      // For all kernel modules
#include <linux/kdev_t.h>      // For device numbers
#include <linux/fs.h>          // For file_operations
#include <linux/cdev.h>        // For cdev structure
#include <linux/device.h>      // For class and device creation
#include <linux/slab.h>        // For kmalloc() and kfree()
#include <linux/uaccess.h>     // For copy_to_user()/copy_from_user()
#include <linux/workqueue.h>   // For dynamic workqueues

/* ---------- Device variables ---------- */
dev_t dev = 0;                 // Device number (major + minor)
static struct class *dev_class; // Device class pointer
static struct cdev dev_cdev;    // Character device structure

/* ---------- Workqueue ---------- */
static struct work_struct workqueue; // Workqueue structure

/* Workqueue function executed in process context */
static void workqueue_fn(struct work_struct *work)
{
    printk(KERN_INFO "etx: Executing Workqueue Function\n");
}

/* ---------- File Operations ---------- */

/* Device open callback */
static int dev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "etx: Device File Opened\n");
    return 0;
}

/* Device release callback */
static int dev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "etx: Device File Closed\n");
    return 0;
}

/* Device read callback - schedules workqueue */
static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "etx: Read called - scheduling workqueue\n");
    schedule_work(&workqueue); // Queue work to be executed later
    return 0;
}

/* Device write callback - also schedules workqueue */
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "etx: Write called - scheduling workqueue\n");
    schedule_work(&workqueue); // Queue work to be executed later
    return len;
}

/* File operations structure */
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = dev_open,
    .release = dev_release,
    .read    = dev_read,
    .write   = dev_write,
};

/* ---------- Module Initialization ---------- */
static int __init dev_driver_init(void)
{
    int ret;

    /* Allocate major/minor device number dynamically */
    ret = alloc_chrdev_region(&dev, 0, 1, "dev_Dev");
    if (ret < 0) {
        printk(KERN_ERR "dev: Cannot allocate major number\n");
        return ret;
    }
    printk(KERN_INFO "dev: Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    /* Initialize cdev structure and add to kernel */
    cdev_init(&dev_cdev, &fops);
    ret = cdev_add(&dev_cdev, dev, 1);
    if (ret < 0) {
        printk(KERN_ERR "dev: Cannot add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    /* Create device class */
    dev_class = class_create(THIS_MODULE, "dev_class");
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR "dev: Cannot create class\n");
        cdev_del(&dev_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(dev_class);
    }

    /* Create device node /dev/etx_device */
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device"))) {
        printk(KERN_ERR "dev: Cannot create device\n");
        class_destroy(dev_class);
        cdev_del(&dev_cdev);
        unregister_chrdev_region(dev, 1);
        return -EINVAL;
    }

    /* Initialize dynamic workqueue with callback function */
    INIT_WORK(&workqueue, workqueue_fn);

    printk(KERN_INFO "etx: Device Driver Insert...Done!!!\n");
    return 0;
}

/* ---------- Module Exit ---------- */
static void __exit dev_driver_exit(void)
{
    flush_scheduled_work();  // Ensure any queued work is finished before exit

    device_destroy(dev_class, dev); // Remove device node
    class_destroy(dev_class);       // Destroy device class
    cdev_del(&dev_cdev);            // Delete cdev
    unregister_chrdev_region(dev, 1); // Release major/minor numbers

    printk(KERN_INFO "etx: Device Driver Remove...Done!!!\n");
}

/* Register init and exit functions */
module_init(dev_driver_init);
module_exit(dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Device driver - dynamic workqueue example");
