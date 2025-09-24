#include <linux/kernel.h>      // For printk()
#include <linux/init.h>        // For module_init, module_exit
#include <linux/module.h>      // For all kernel modules
#include <linux/kdev_t.h>      // For device numbers
#include <linux/fs.h>          // For file operations
#include <linux/cdev.h>        // For cdev utilities
#include <linux/device.h>      // For device_create/destroy
#include <linux/slab.h>        // For kmalloc/kfree
#include <linux/uaccess.h>     // For copy_to_user/copy_from_user
#include <linux/sysfs.h>       // For sysfs support
#include <linux/kobject.h>     // For kobject
#include <linux/interrupt.h>   // For IRQ handling
#include <asm/io.h>            // For io access (if needed)
#include <linux/err.h>         // For error handling

#define IRQ_NO 11              // Example IRQ number

volatile int etx_value = 0;    // Value exposed via sysfs

dev_t dev = 0;                 // Device number (major+minor)
static struct class *dev_class; // Device class
static struct cdev etx_cdev;    // Character device structure
static struct kobject *kobj_ref; // Sysfs kobject reference

/* ---------- Interrupt Handler ---------- */
// This is called whenever IRQ 11 is triggered.
static irqreturn_t irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "etx: Shared IRQ - Interrupt occurred (irq=%d)\n", irq);
    // Can set flags, wake up processes, notify userspace, etc.
    return IRQ_HANDLED;
}

/* ---------- Sysfs show (read) ---------- */
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    printk(KERN_INFO "etx: Sysfs - Read\n");
    return sprintf(buf, "%d\n", etx_value);  // Return value to userspace
}

/* ---------- Sysfs store (write) ---------- */
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    printk(KERN_INFO "etx: Sysfs - Write\n");
    sscanf(buf, "%d", &etx_value);           // Store written value
    return count;
}

// Sysfs attribute structure
struct kobj_attribute etx_attr = __ATTR(etx_value, 0660, sysfs_show, sysfs_store);

/* ---------- File Operations ---------- */
static int etx_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "etx: Device File Opened\n");
    return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "etx: Device File Closed\n");
    return 0;
}

/* Simulate an IRQ from read() — useful for testing */
static void simulate_irq(void)
{
    irq_handler(IRQ_NO, &etx_cdev);  // Call handler manually
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "etx: Read function called - simulating IRQ\n");
    simulate_irq();   // Simulate interrupt event
    return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "etx: Write function\n");
    return len;
}

// File operations structure
static struct file_operations fops =
{
    .owner   = THIS_MODULE,
    .read    = etx_read,
    .write   = etx_write,
    .open    = etx_open,
    .release = etx_release,
};

/* ---------- Module Init ---------- */
static int __init etx_driver_init(void)
{
    int ret;

    // Allocate device number (major+minor)
    ret = alloc_chrdev_region(&dev, 0, 1, "etx_Dev");
    if (ret < 0) {
        printk(KERN_ERR "etx: Cannot allocate major number\n");
        return ret;
    }
    printk(KERN_INFO "etx: Major = %d Minor = %d\n", MAJOR(dev), MINOR(dev));

    // Initialize and add cdev
    cdev_init(&etx_cdev, &fops);
    ret = cdev_add(&etx_cdev, dev, 1);
    if (ret) {
        printk(KERN_ERR "etx: cdev_add failed\n");
        goto fail_unreg;
    }

    // Create class
    dev_class = class_create(THIS_MODULE, "etx_class");
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR "etx: Cannot create the struct class\n");
        ret = PTR_ERR(dev_class);
        goto fail_cdev;
    }

    // Create device node in /dev/
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device"))) {
        printk(KERN_ERR "etx: Cannot create the Device\n");
        ret = -EINVAL;
        goto fail_class;
    }

    // Create sysfs directory /sys/kernel/etx_sysfs/
    kobj_ref = kobject_create_and_add("etx_sysfs", kernel_kobj);
    if (!kobj_ref) {
        printk(KERN_ERR "etx: kobject_create_and_add failed\n");
        ret = -ENOMEM;
        goto fail_device;
    }

    // Create sysfs attribute file etx_value
    if (sysfs_create_file(kobj_ref, &etx_attr.attr)) {
        printk(KERN_ERR "etx: Cannot create sysfs file\n");
        ret = -EFAULT;
        goto fail_kobj;
    }

    // Register IRQ handler (shared IRQ, dev_id = &etx_cdev)
    ret = request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "etx_device", &etx_cdev);
    if (ret) {
        printk(KERN_ERR "etx: cannot register IRQ %d, ret=%d\n", IRQ_NO, ret);
        goto fail_sysfs;
    }

    printk(KERN_INFO "etx: Device Driver Insert...Done!!!\n");
    return 0;

/* Cleanup on errors (reverse order) */
fail_sysfs:
    sysfs_remove_file(kobj_ref, &etx_attr.attr);
fail_kobj:
    kobject_put(kobj_ref);
fail_device:
    device_destroy(dev_class, dev);
fail_class:
    class_destroy(dev_class);
fail_cdev:
    cdev_del(&etx_cdev);
fail_unreg:
    unregister_chrdev_region(dev, 1);
    return ret;
}

/* ---------- Module Exit ---------- */
static void __exit etx_driver_exit(void)
{
    // Free IRQ
    free_irq(IRQ_NO, &etx_cdev);

    // Cleanup sysfs
    sysfs_remove_file(kobj_ref, &etx_attr.attr);
    kobject_put(kobj_ref);

    // Destroy device and class
    device_destroy(dev_class, dev);
    class_destroy(dev_class);

    // Remove cdev and unregister device number
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "etx: Device Driver Remove...Done!!!\n");
}

/* ---------- Register Module Init/Exit ---------- */
module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Device driver - interrupt example");
