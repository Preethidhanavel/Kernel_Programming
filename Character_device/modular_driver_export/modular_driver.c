#include <linux/init.h>
 #include <linux/module.h>
 #include <linux/fs.h>
 #include <linux/uaccess.h>
 #include <linux/cdev.h>
 #include <linux/device.h>
 
 #define DEVICE_NAME "devmod"
 #define CLASS_NAME "devclass"
 #define BUF_LEN 128
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("You");
 MODULE_DESCRIPTION("Device driver using core module");
 
 static dev_t dev_num;
 static struct cdev dev_cdev;
 static struct class *dev_class;
 static struct device *dev_device;
 
 extern void core_log_message(const char *msg); // Declare function from core
 
 static char buffer[BUF_LEN];
static char device_buffer[256];
static int data_size = 0;

ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    int ret;

    if (*ppos >= data_size) {
        // No more data, return EOF
        return 0;
    }

    // Adjust count if user asks for more than available
    if (count > data_size - *ppos)
        count = data_size - *ppos;

    // Copy data to user
    ret = copy_to_user(buf, device_buffer + *ppos, count);
    if (ret != 0) {
        return -EFAULT; // Copy failed
    }

    *ppos += count;  // Move file pointer
    return count;    // Number of bytes read
} 
 static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
 {
 
    int ret;

    if (count > sizeof(device_buffer))
        count = sizeof(device_buffer);

    ret = copy_from_user(device_buffer, buf, count);
    if (ret != 0)
        return -EFAULT;

    data_size = count;
    *ppos = 0;   // reset file pointer
    return count;
 }
 
 static int dev_open(struct inode *inode, struct file *file) 
{ 
	return 0; 
}
 static int dev_release(struct inode *inode, struct file *file) 
{
       	return 0; 
}
 
 static struct file_operations fops = {
 .owner = THIS_MODULE,
 .write = dev_write,
 .open = dev_open,
 .read = my_read,
 .release = dev_release,
};
 
 static int __init device_init(void)
 {
 int ret;
 
 ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
 if (ret < 0) return ret;
 
 cdev_init(&dev_cdev, &fops);
 cdev_add(&dev_cdev, dev_num, 1);
 
 dev_class = class_create(THIS_MODULE, CLASS_NAME);
 dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
 
 printk(KERN_INFO "[%s] device loaded\n", DEVICE_NAME);
 return 0;
 }
 
 static void __exit device_exit(void)
 {
 device_destroy(dev_class, dev_num);
 class_destroy(dev_class);
 cdev_del(&dev_cdev);
 unregister_chrdev_region(dev_num, 1);
 printk(KERN_INFO "[%s] device unloaded\n", DEVICE_NAME);
 }
 
 
 
 module_init(device_init);
 module_exit(device_exit);
