#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/uaccess.h>
#include<linux/ioctl.h>
#include"ioctl_data.h"

#define IOCTL_GET_RESULT _IOWR('a','b',struct data)
static char *device_name = "mydev1";
static char * class_name = "myclass1";
static dev_t dev_number;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;

static int dev_open(struct inode *inode, struct file*file)
{
printk(KERN_INFO"[%s] opened\n", device_name);
return 0;
}

static int dev_release(struct inode*inode,struct file*file)
{
printk(KERN_INFO"[%s] closed\n", device_name);
return 0;
}



static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
struct data udata;
int i;
switch (cmd)
{
case IOCTL_GET_RESULT: 
       	if(copy_from_user(&udata, (struct data *)arg,sizeof(udata)))
{
return -EFAULT;
}
udata.max = udata.arr[0];
udata.min = udata.arr[0];
for(i=0;i<udata.size;i++)
{
if(udata.max<udata.arr[i])
udata.max=udata.arr[i];
if(udata.min>udata.arr[i])
udata.min=udata.arr[i];
}
	printk("MAX = %d  MIN = %d\n",udata.max,udata.min);
if(copy_to_user((struct data *)arg, &udata,sizeof(udata)))
	return -EFAULT;

break;

default:
  return -EINVAL;
}
return 0;
}


static struct file_operations fops={
.owner = THIS_MODULE,
.open = dev_open,
.unlocked_ioctl = dev_ioctl,
.release = dev_release,
};

static int __init dev_init(void)
{
alloc_chrdev_region(&dev_number , 0,1,device_name);
cdev_init(&my_cdev,&fops);
cdev_add(&my_cdev , dev_number ,1);
my_class = class_create(THIS_MODULE,class_name);
my_device = device_create(my_class,NULL,dev_number,NULL,device_name);
printk(KERN_INFO"character device file created\n");

return 0;
}

static void __exit dev_exit(void)
{

device_destroy(my_class,dev_number);
class_destroy(my_class);
cdev_del(&my_cdev);
unregister_chrdev_region(dev_number,1);
printk(KERN_INFO"character device removed\n");
}

module_init(dev_init);
module_exit(dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Preethi");
MODULE_DESCRIPTION("character device file");
