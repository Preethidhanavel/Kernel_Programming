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
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/err.h>

DECLARE_WAIT_QUEUE_HEAD(wait_queue_dev_data);

dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;
struct kobject *kobj_ref;

static bool can_write = false;
static bool can_read  = false;
static char dev_value[20];

static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
  pr_info("Sysfs Show - Write Permission Granted!!!\n");
  can_write = true;
  wake_up(&wait_queue_dev_data);
  return sprintf(buf, "%s", "Success\n");
}

static ssize_t sysfs_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
  pr_info("Sysfs Store - Read Permission Granted!!!\n");
  strcpy(dev_value, buf);
  can_read = true;
  wake_up(&wait_queue_dev_data);
  return count;
}

struct kobj_attribute dev_attr = __ATTR(dev_value, 0660, sysfs_show, sysfs_store);

static int dev_open(struct inode *inode, struct file *file)
{
  pr_info("Device File Opened...!!!\n");
  return 0;
}

static int dev_release(struct inode *inode, struct file *file)
{
  pr_info("Device File Closed...!!!\n");
  return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
  pr_info("Read Function : dev_value = %s\n",dev_value);   
  len = strlen(dev_value);
  strcpy(buf, dev_value);
  return 0;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
  strcpy(dev_value, buf);
  pr_info("Write function : dev_value = %s\n", dev_value);
  return len;
}

static unsigned int dev_poll(struct file *filp, struct poll_table_struct *wait)
{
  __poll_t mask = 0;
  poll_wait(filp, &wait_queue_dev_data, wait);
  pr_info("Poll function\n");
  if( can_read )
  {
    can_read = false;
    mask |= ( POLLIN | POLLRDNORM );
  }
  if( can_write )
  {
    can_write = false;
    mask |= ( POLLOUT | POLLWRNORM );
  }
  return mask;
}

static struct file_operations fops =
{
  .owner          = THIS_MODULE,
  .read           = dev_read,
  .write          = dev_write,
  .open           = dev_open,
  .release        = dev_release,
  .poll           = dev_poll
};
 
static int __init dev_driver_init(void)
{
  if((alloc_chrdev_region(&dev, 0, 1, "dev_Dev")) <0)
  {
    pr_err("Cannot allocate major number\n");
    return -1;
  }
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
  cdev_init(&dev_cdev,&fops);
  dev_cdev.owner = THIS_MODULE;
  dev_cdev.ops = &fops;
  if((cdev_add(&dev_cdev,dev,1)) < 0)
  {
    pr_err("Cannot add the device to the system\n");
    goto r_class;
  }
  if(IS_ERR(dev_class = class_create(THIS_MODULE,"dev_class")))
  {
    pr_err("Cannot create the struct class\n");
    goto r_class;
  }
  if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"dev_device")))
  {
    pr_err("Cannot create the Device 1\n");
    goto r_device;
  }
  kobj_ref = kobject_create_and_add("dev_sysfs",kernel_kobj);
  if(sysfs_create_file(kobj_ref,&dev_attr.attr))
  {
    printk(KERN_INFO"Cannot create sysfs file......\n");
    goto r_sysfs;
  }
  pr_info("Device Driver Insert...Done!!!\n");
  return 0;

r_sysfs:
  kobject_put(kobj_ref); 
  sysfs_remove_file(kernel_kobj, &dev_attr.attr);
r_device:
  class_destroy(dev_class);
r_class:
  unregister_chrdev_region(dev,1);
  return -1;
}

static void __exit dev_driver_exit(void)
{
  kobject_put(kobj_ref); 
  sysfs_remove_file(kernel_kobj, &dev_attr.attr);
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
MODULE_DESCRIPTION("Device driver select");

