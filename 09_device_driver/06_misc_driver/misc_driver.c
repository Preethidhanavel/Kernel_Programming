#include <linux/miscdevice.h>   // for misc device framework
#include <linux/fs.h>           // for file operations structure
#include <linux/kernel.h>       // for pr_info, pr_err
#include <linux/module.h>       // for module macros
#include <linux/init.h>         // for __init, __exit
#include <linux/uaccess.h>      // for copy_to_user, copy_from_user
#include <linux/slab.h>         // for kmalloc, kfree

#define KBUF_SIZE 1024          // buffer size (1 KB)

static char *k_buf;             // kernel buffer pointer

// Called when user opens the device file
static int dev_misc_open(struct inode *inode, struct file *file)
{
    pr_info("dev misc device open\n");
    return 0;
}

// Called when user closes the device file
static int dev_misc_close(struct inode *inodep, struct file *filp)
{
    pr_info("dev misc device close\n");
    return 0;
}

// Called when user writes data to device file
static ssize_t dev_misc_write(struct file *file, const char __user *buf,
                              size_t len, loff_t *ppos)
{
    // copy only as much as buffer allows (leave space for '\0')
    size_t to_copy = min(len, (size_t)(KBUF_SIZE - 1));

    pr_info("dev misc device write\n");

    // copy data from user-space buffer to kernel buffer
    if (copy_from_user(k_buf, buf, to_copy))
        return -EFAULT;   // return error if copy fails

    k_buf[to_copy] = '\0';  // null-terminate string

    return to_copy;         // return number of bytes written
}

// Called when user reads data from device file
static ssize_t dev_misc_read(struct file *filp, char __user *buf,
                             size_t count, loff_t *f_pos)
{
    size_t len = strlen(k_buf);   // length of data in kernel buffer

    pr_info("dev misc device read\n");

    // copy data from kernel buffer to user buffer
    return simple_read_from_buffer(buf, count, f_pos, k_buf, len);
}

// File operations table (links syscalls to our functions)
static const struct file_operations fops = {
    .owner          = THIS_MODULE,
    .write          = dev_misc_write,
    .read           = dev_misc_read,
    .open           = dev_misc_open,
    .release        = dev_misc_close,
    .llseek         = no_llseek,   // no seeking support
};

// Misc device structure
static struct miscdevice dev_misc_device = {
    .minor = MISC_DYNAMIC_MINOR,   // assign minor dynamically
    .name = "simple_dev_misc",     // will appear as /dev/simple_dev_misc
    .fops = &fops,                 // link file operations
};

// Module init function (runs when module is inserted)
static int __init misc_init(void)
{
    int error;

    // allocate memory for kernel buffer
    k_buf = kmalloc(KBUF_SIZE, GFP_KERNEL);
    if (!k_buf)
        return -ENOMEM;

    // register misc device
    error = misc_register(&dev_misc_device);
    if (error) {
        pr_err("misc_register failed!!!\n");
        kfree(k_buf);   // free buffer if registration fails
        return error;
    }

    pr_info("misc is successfully registered!!!\n");
    return 0;
}

// Module exit function (runs when module is removed)
static void __exit misc_exit(void)
{
    misc_deregister(&dev_misc_device);  // unregister device
    kfree(k_buf);                       // free allocated buffer
    pr_info("misc is unregistered!!!\n");
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");                  // License
MODULE_AUTHOR("PREETHI");               // Author name
MODULE_DESCRIPTION("Misc Driver");      // Description
