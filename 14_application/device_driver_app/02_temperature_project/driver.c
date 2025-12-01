#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/uaccess.h>

// IOCTL command for checking temperature range
#define THRESHOLD_CHECK _IOWR('a', 0x11, int)

// Return values for user space application
#define TH_HIGH 0x22
#define TH_LOW  0x33
#define TH_WITH_IN_LIMIT 0x44

// Values sent back to user based on temperature comparison
int th_high = 0x22, th_low = 0x33, th_with_limit = 0x44;

// External function - implemented in another kernel component
extern int get_temp_val(void);

struct class *cl;
struct device *device_node;
dev_t dev_num;
struct cdev my_dev;

int temp = 0;

// User-configurable threshold values passed via module parameters
int threshold_high = 0;
int threshold_low = 0;

module_param(threshold_high, int , 0644);
module_param(threshold_low, int , 0644);

//----------------------------------------------------
// File read function - sends latest temperature value to user space
//----------------------------------------------------
static ssize_t device_read(struct file *fp, char __user *usr_buf, size_t len, loff_t *off)
{
    pr_info("Read request received\n");

    // Get latest temperature from external function
    temp = get_temp_val();

    // Send temperature to user-space
    copy_to_user((int *)usr_buf, &temp, sizeof(temp));

    return sizeof(temp);
}

//----------------------------------------------------
// File write function (Not used, only prints log)
//----------------------------------------------------
static ssize_t device_write(struct file *fp, const char __user *usr_buf, size_t len, loff_t *off)
{
    pr_info("Write operation invoked (Not used)\n");
    return len;
}

//----------------------------------------------------
// Open function - called when /dev/mydevice is opened
//----------------------------------------------------
static int device_open(struct inode *inode, struct file *file)
{
    pr_info("Device opened\n");
    return 0;
}

//----------------------------------------------------
// Release function - called when file is closed
//----------------------------------------------------
static int device_release(struct inode *inode, struct file *file)
{
    pr_info("Device closed\n");
    return 0;
}

//----------------------------------------------------
// IOCTL handler - compares temperature with threshold values
//----------------------------------------------------
static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch(cmd)
    {
        case THRESHOLD_CHECK:
            if(temp > threshold_high)
                copy_to_user((int __user *)arg, &th_high, sizeof(int));
            else if(temp < threshold_low)
                copy_to_user((int __user *)arg, &th_low, sizeof(int));
            else
                copy_to_user((int __user *)arg, &th_with_limit, sizeof(int));
            break;

        default:
            pr_info("Invalid IOCTL command\n");
            return -EINVAL;
    }
    return 0;
}

//----------------------------------------------------
// File operations structure
//----------------------------------------------------
struct file_operations fop = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};

//----------------------------------------------------
// Module initialization
//----------------------------------------------------
static int __init temp_init(void)
{
    // Create device class
    cl = class_create("myclass");

    // Allocate major & minor numbers dynamically
    if(!alloc_chrdev_region(&dev_num, 0 , 1, "char_driver"))
    {
        printk("Device number registered\n");
        printk("Major: %d  Minor: %d\n", MAJOR(dev_num), MINOR(dev_num));

        // Register character device with fops
        cdev_init(&my_dev, &fop);
        cdev_add(&my_dev, dev_num, 1);

        // Create device node under /dev/mydevice
        device_node = device_create(cl, NULL, dev_num, NULL, "mydevice");

        pr_info("Module loaded successfully\n");
        pr_info("Initial Temperature: %d\n", get_temp_val());
    }
    else
    {
        pr_info("Failed to allocate device number\n");
    }
    return 0;
}

//----------------------------------------------------
// Module cleanup
//----------------------------------------------------
static void __exit temp_exit(void)
{
    device_destroy(cl, dev_num);
    class_destroy(cl);
    cdev_del(&my_dev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "Module unloaded\n");
}

module_init(temp_init);
module_exit(temp_exit);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Temperature monitoring driver using IOCTL");
MODULE_AUTHOR("Preethi");
