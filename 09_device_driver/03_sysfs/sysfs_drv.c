#include <linux/kernel.h>      // Kernel header
#include <linux/init.h>        // For module init/exit macros
#include <linux/module.h>      // For all modules
#include <linux/kdev_t.h>      // For device numbers
#include <linux/fs.h>          // File operations structure
#include <linux/cdev.h>        // Character device structure
#include <linux/device.h>      // For device creation
#include <linux/slab.h>        // For kmalloc/kfree
#include <linux/uaccess.h>     // For copy_to_user/copy_from_user
#include <linux/sysfs.h>       // For sysfs support
#include <linux/kobject.h>     // For kobject
#include <linux/err.h>         // For error handling
 
volatile int dev_value = 0;    // Variable exposed to sysfs
 
dev_t dev = 0;                 // Device number (major + minor)
static struct class *dev_class; // Device class
static struct cdev dev_cdev;    // Character device structure
struct kobject *kobj_ref;       // Kobject reference

// Callback when reading from sysfs file
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        pr_info("Sysfs - Read!!!\n");
        return sprintf(buf, "%d", dev_value);   // Return dev_value to user
}

// Callback when writing to sysfs file
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        pr_info("Sysfs - Write!!!\n");
        sscanf(buf, "%d", &dev_value);   // Store user input into dev_value
        return count;
}

// Define sysfs attribute (name = dev_value, permissions = 0660)
struct kobj_attribute dev_attr = __ATTR(dev_value, 0660, sysfs_show, sysfs_store);

// File open callback
static int dev_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

// File release callback
static int dev_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

// File read callback
static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read function\n");
        return 0;
}

// File write callback
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write Function\n");
        return len;
}

// File operations structure
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = dev_read,
        .write          = dev_write,
        .open           = dev_open,
        .release        = dev_release,
};

// Module init function
static int __init dev_driver_init(void)
{
        // Allocate major and minor numbers dynamically
        if ((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) < 0) {
                pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
 
        // Initialize cdev structure
        cdev_init(&dev_cdev, &fops);
 
        // Add cdev to kernel
        if ((cdev_add(&dev_cdev, dev, 1)) < 0) {
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }
 
        // Create device class
        if (IS_ERR(dev_class = class_create(THIS_MODULE, "dev_class"))) {
            pr_info("Cannot create the struct class\n");
            goto r_class;
        }
 
        // Create device node in /dev/
        if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device"))) {
            pr_info("Cannot create the Device 1\n");
            goto r_device;
        }
 
        // Create kobject under /sys/kernel/
        kobj_ref = kobject_create_and_add("dev_sysfs", kernel_kobj);
 
        // Create sysfs file (dev_value) inside /sys/kernel/dev_sysfs/
        if (sysfs_create_file(kobj_ref, &dev_attr.attr)) {
                pr_err("Cannot create sysfs file......\n");
                goto r_sysfs;
        }

        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
 
// Error handling cleanup
r_sysfs:
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &dev_attr.attr);
 
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
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &dev_attr.attr);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        cdev_del(&dev_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
// Register init and exit functions
module_init(dev_driver_init);
module_exit(dev_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("sysfs integration");
