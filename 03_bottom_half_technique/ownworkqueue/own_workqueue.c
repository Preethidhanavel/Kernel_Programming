#include <linux/kernel.h>      // For printk()
#include <linux/init.h>        // For module_init, module_exit
#include <linux/module.h>      // For all kernel modules
#include <linux/kdev_t.h>      // For device numbers
#include <linux/fs.h>          // For file_operations
#include <linux/cdev.h>        // For cdev
#include <linux/device.h>      // For device class and device node
#include <linux/slab.h>        // For kmalloc/kfree
#include <linux/uaccess.h>     // For copy_to_user / copy_from_user
#include <linux/workqueue.h>   // For workqueue

/* ---------- Character Device Variables ---------- */
dev_t dev = 0;                    // Device number (major+minor)
static struct class *dev_class;   // Device class pointer
static struct cdev etx_cdev;      // Character device structure
static uint8_t k_buf[100];        // Kernel buffer for read/write
/* ---------- Workqueue ---------- */
static struct workqueue_struct *own_workqueue; // Dedicated workqueue pointer

/* Work structure */
static void workqueue_fn(struct work_struct *work)
{
    printk(KERN_INFO "Workqueue function executed\n");
}

/* Declare a work item */
static DECLARE_WORK(work, workqueue_fn);

/* ---------- File Operations ---------- */

/* Open function */
static int etx_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device file opened\n");
    return 0;
}

/* Release function */
static int etx_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device file closed\n");
    return 0;
}

/* Read function - schedules workqueue and copies data to userspace */
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Read called - scheduling workqueue\n");

    queue_work(own_workqueue, &work);   // Queue work to dedicated workqueue
    return simple_read_from_buffer(buf, len, off, k_buf, strlen(k_buf));
}

/* Write function - schedules workqueue and stores data from userspace */
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "Write called - scheduling workqueue\n");

    queue_work(own_workqueue, &work);   // Queue work to dedicated workqueue
    copy_from_user(k_buf, buf, len);    // Copy data from userspace
    k_buf[len] = '\0';                  // Null terminate
    return len;
}

/* File operations structure */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = etx_open,
    .release = etx_release,
    .read = etx_read,
    .write = etx_write,
};

/* ---------- Module Initialization ---------- */
static int __init etx_driver_init(void)
{
    /* Allocate major/minor device number dynamically */
    if (alloc_chrdev_region(&dev, 0, 1, "etx_device") < 0) {
        printk(KERN_ERR "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    /* Initialize cdev and add to kernel */
    cdev_init(&etx_cdev, &fops);
    if (cdev_add(&etx_cdev, dev, 1) < 0) {
        printk(KERN_ERR "Cannot add cdev\n");
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    /* Create device class */
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "etx_class"))) {
        printk(KERN_ERR "Cannot create class\n");
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    /* Create device node /dev/etx_device */
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device"))) {
        printk(KERN_ERR "Cannot create device\n");
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    /* Create dedicated workqueue */
    own_workqueue = create_workqueue("own_wq");

    printk(KERN_INFO "Driver initialized successfully\n");
    return 0;
}

/* ---------- Module Exit ---------- */
static void __exit etx_driver_exit(void)
{
    destroy_workqueue(own_workqueue);     // Destroy dedicated workqueue
    device_destroy(dev_class, dev);       // Remove device node
    class_destroy(dev_class);             // Destroy device class
    cdev_del(&etx_cdev);                  // Delete cdev
    unregister_chrdev_region(dev, 1);     // Release major/minor numbers

    printk(KERN_INFO "Driver removed\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("device driver with own workqueue");

