#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 // kmalloc()
#include <linux/uaccess.h>              // copy_to/from_user()
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/sched/signal.h>

#define SIGdev 44                        // Custom signal number sent to user app
#define REG_CURRENT_TASK _IOW('s',1,int32_t*) // IOCTL to register current task
#define IRQ_NO 11                        // IRQ number for interrupt simulation

static struct task_struct *task = NULL;   // User-space task to signal
static int signum = 0;
int32_t value = 0;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;

// IRQ handler for IRQ 11
static irqreturn_t irq_handler(int irq, void *dev_id) {
    struct kernel_siginfo info;

    printk(KERN_INFO "Shared IRQ: Interrupt Occurred\n");

    // Prepare signal info to send to user application
    memset(&info, 0, sizeof(struct kernel_siginfo));
    info.si_signo = SIGdev;
    info.si_code = SI_QUEUE;
    info.si_int = 1;

    // Send signal to registered user task
    if (task != NULL) {
        printk(KERN_INFO "Sending signal to app\n");
        if (send_sig_info(SIGdev, &info, task) < 0) {
            printk(KERN_INFO "Unable to send signal\n");
        }
    }

    return IRQ_HANDLED;
}

// Called when device file is opened
static int dev_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "Device File Opened...!!!\n");
    return 0;
}

// Called when device file is closed
static int dev_release(struct inode *inode, struct file *file) {
    struct task_struct *ref_task = get_current();
    printk(KERN_INFO "Device File Closed...!!!\n");

    // Unregister the task if it was registered
    if (ref_task == task) {
        task = NULL;
    }
    return 0;
}

// Simulate an interrupt by manually calling IRQ handler
static void simulate_irq(void) {
    irq_handler(IRQ_NO, &dev_cdev);
}

// Called when device is read
static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Read Function\n");
    simulate_irq(); // Trigger simulated interrupt
    return 0;
}

// Called when device is written
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Write function\n");
    return 0;
}

// IOCTL function to register the current user task for signaling
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    if (cmd == REG_CURRENT_TASK) {
        printk(KERN_INFO "REG_CURRENT_TASK\n");
        task = get_current();   // Save current task
        signum = SIGdev;        // Save signal number
    }
    return 0;
}

// File operations structure
static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .read           = dev_read,
    .write          = dev_write,
    .open           = dev_open,
    .unlocked_ioctl = dev_ioctl,
    .release        = dev_release,
};

// Module init function
static int __init dev_driver_init(void) {
    // Allocate device number
    if ((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) < 0) {
        printk(KERN_INFO "Cannot allocate major number\n");
        return -1;
    }
    printk(KERN_INFO "Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

    // Initialize cdev
    cdev_init(&dev_cdev, &fops);

    // Add cdev to system
    if ((cdev_add(&dev_cdev, dev, 1)) < 0) {
        printk(KERN_INFO "Cannot add the device to the system\n");
        goto r_class;
    }

    // Create device class
    if (IS_ERR(dev_class = class_create(THIS_MODULE, "dev_class"))) {
        printk(KERN_INFO "Cannot create the struct class\n");
        goto r_class;
    }

    // Create device
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "dev_device"))) {
        printk(KERN_INFO "Cannot create the Device 1\n");
        goto r_device;
    }

    // Request IRQ
    if (request_irq(IRQ_NO, irq_handler, IRQF_SHARED, "dev_device", (void *)(irq_handler))) {
        printk(KERN_INFO "my_device: cannot register IRQ\n");
        goto irq;
    }

    printk(KERN_INFO "Device Driver Insert...Done!!!\n");
    return 0;

irq:
    free_irq(IRQ_NO, (void *)(irq_handler));
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev, 1);
    return -1;
}

// Module exit function
static void __exit dev_driver_exit(void) {
    free_irq(IRQ_NO, (void *)(irq_handler));
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "Device Driver Remove...Done!!!\n");
}

module_init(dev_driver_init);
module_exit(dev_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("device driver - Signal");
