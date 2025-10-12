#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>

#define IRQ_NO 11  // IRQ number for demonstration

volatile int etx_value = 0; // Shared variable

dev_t dev = 0;                 // Device number (major+minor)
static struct class *dev_class; // Device class
static struct cdev etx_cdev;    // Character device structure

/* ---------- Dynamic Tasklet ---------- */
static struct tasklet_struct my_tasklet; // Declare dynamic tasklet

/* Tasklet function executed in softirq context */
static void tasklet_fn(unsigned long data)
{
    printk(KERN_INFO "Dynamic Tasklet executed with data: %lu\n", data);
}

/* ---------- File Operations ---------- */

/* Device open callback */
static int etx_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "etx: Device Opened\n");
    return 0;
}

/* Device release callback */
static int etx_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "etx: Device Closed\n");
    return 0;
}

/* IRQ handler - schedules dynamic tasklet */
static irqreturn_t irq_handler(int irq, void *dev_id)
{
    printk(KERN_INFO "etx: IRQ %d occurred, scheduling dynamic tasklet\n", irq);
    tasklet_schedule(&my_tasklet); // Queue dynamic tasklet
    return IRQ_HANDLED;
}

/* Simulate an IRQ for testing */
static void simulate_irq(void)
{
    irq_handler(IRQ_NO, &etx_cdev);
}

/* Device read callback - triggers simulated IRQ */
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "etx: Read called\n");
    simulate_irq(); // simulate hardware IRQ
    return 0;
}

/* Device write callback */
static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "etx: Write called\n");
    return len;
}

/* File operations structure */
static struct file_operations fops = {
    .owner   = THIS_MODULE,
    .open    = etx_open,
    .release = etx_release,
    .read    = etx_read,
    .write   = etx_write,
};

/* ---------- Module Initialization ---------- */
static int __init etx_driver_init(void)
{
    int ret;

    /* Allocate major/minor number */
    ret = alloc_chrdev_region(&dev, 0, 1, "etx_device");
    if (ret < 0) {
        printk(KERN_ERR "etx: Failed to allocate major number\n");
        return ret;
    }
    printk(KERN_INFO "etx: Major=%d Minor=%d\n", MAJOR(dev), MINOR(dev));

    /* Initialize cdev and add to kernel */
    cdev_init(&etx_cdev, &fops);
    ret = cdev_add(&etx_cdev, dev, 1);
    if (ret) {
        printk(KERN_ERR "etx: cdev_add failed\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    /* Create device class */
    dev_class = class_create(THIS_MODULE, "etx_class");
    if (IS_ERR(dev_class)) {
        printk(KERN_ERR "etx: Failed to create class\n");
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(dev_class);
    }

    /* Create device node /dev/etx_device */
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "etx_device"))) {
        printk(KERN_ERR "etx: Failed to create device\n");
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return -EINVAL;
    }

    /* Initialize dynamic tasklet with data = 123 */
    tasklet_init(&my_tasklet, tasklet_fn, 123);

    /* Request IRQ */
    ret = request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "etx_device", &etx_cdev);
    if (ret) {
        printk(KERN_ERR "etx: Cannot register IRQ %d, ret=%d\n", IRQ_NO, ret);
        tasklet_kill(&my_tasklet); // cleanup tasklet
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        cdev_del(&etx_cdev);
        unregister_chrdev_region(dev, 1);
        return ret;
    }

    printk(KERN_INFO "etx: Driver loaded (IRQ + dynamic tasklet)\n");
    return 0;
}

/* ---------- Module Exit ---------- */
static void __exit etx_driver_exit(void)
{
    free_irq(IRQ_NO, &etx_cdev); // Free IRQ
    tasklet_kill(&my_tasklet);   // Kill dynamic tasklet if scheduled
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&etx_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "etx: Driver unloaded\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("device driver with tasklet dynamic creation");
MODULE_VERSION("1.1");
