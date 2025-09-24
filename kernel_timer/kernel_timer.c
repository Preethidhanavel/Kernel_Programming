#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/err.h>

//Timer Variable
#define TIMEOUT 5000    //milliseconds

static struct timer_list dev_timer;
static unsigned int count = 0; 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;
 
//Timer Callback function. This will be called when timer expires
void timer_callback(struct timer_list * data)
{
    /* do your timer stuff here */
    pr_info("Timer Callback function Called [%d]\n",count++);
    
    /*
       Re-enable timer. Because this function will be called only first time. 
       If we re-enable this will work like periodic timer. 
    */
    mod_timer(&dev_timer, jiffies + msecs_to_jiffies(TIMEOUT));
}

/*
** This function will be called when we open the Device file
*/ 
static int dev_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int dev_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t dev_read(struct file *filp,char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t dev_write(struct file *filp,const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}
//File operation structure 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = dev_read,
        .write          = dev_write,
        .open           = dev_open,
        .release        = dev_release,
};
/*
** Module Init function
*/ 
static int __init dev_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) <0){
            pr_err("Cannot allocate major number\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
    /*Creating cdev structure*/
    cdev_init(&dev_cdev,&fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&dev_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }
 
    /*Creating struct class*/
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"dev_class"))){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
 
    /*Creating device*/
    if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"dev_device"))){
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
 
    /* setup your timer to call my_timer_callback */
    timer_setup(&dev_timer, timer_callback, 0);       
 
    /* setup timer interval to based on TIMEOUT Macro */
    mod_timer(&dev_timer, jiffies + msecs_to_jiffies(TIMEOUT));
 
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}


/*
** Module exit function
*/
static void __exit dev_driver_exit(void)
{
    /* remove kernel timer when unloading module */
    del_timer(&dev_timer);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(dev_driver_init);
module_exit(dev_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("device driver - Kernel Timer");

