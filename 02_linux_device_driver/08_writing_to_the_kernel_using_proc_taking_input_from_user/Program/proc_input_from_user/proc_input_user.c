#include <linux/init.h>       // For module init/exit macros
#include <linux/module.h>     // Core header for loading LKMs into the kernel
#include <linux/kernel.h>     // For printk() logging
#include <linux/proc_fs.h>    // For proc filesystem APIs
#include <linux/uaccess.h>    // For copy_to_user() / copy_from_user()

#define PROC_NAME "writer"    // Name of the /proc entry
#define MAX_LEN 100           // Max length of stored message

static char message[MAX_LEN]; // Buffer to hold data written from user-space

// ----------- READ FUNCTION -----------
// Called when user reads from /proc/writer 
ssize_t writer_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    // Copy kernel buffer "message" into user buffer safely
    return simple_read_from_buffer(buf, count, ppos, message, strlen(message));
}

// ----------- WRITE FUNCTION -----------
// Called when user writes into /proc/writer 
ssize_t writer_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    // Prevent writing more than buffer can hold
    if (count > MAX_LEN - 1)
        count = MAX_LEN - 1;

    // Copy data from user space into kernel buffer
    if (copy_from_user(message, buf, count))
        return -EFAULT;   // Return error if copy fails

    message[count] = '\0';  // Null terminate the string

    printk(KERN_INFO "Received from user: %s\n", message);
    return count;           // Return number of bytes written
}

// ----------- FILE OPERATIONS STRUCTURE -----------
// Links read/write operations to our /proc file
static const struct file_operations writer_fops = {
    .owner   = THIS_MODULE,
    .read    = writer_read,   // Called on read
    .write   = writer_write,  // Called on write
};

// ----------- MODULE INIT FUNCTION -----------
// Runs when module is inserted (insmod)
static int __init proc_write_init(void)
{
    proc_create(PROC_NAME, 0666, NULL, &writer_fops);  // Create /proc/writer
    printk(KERN_INFO "/proc/%s created. You can write to it!\n", PROC_NAME);
    return 0;
}

// ----------- MODULE EXIT FUNCTION -----------
// Runs when module is removed (rmmod)
static void __exit proc_write_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);   // Remove /proc/writer
    printk(KERN_INFO "/proc/%s removed.\n", PROC_NAME);
}

// Register module entry/exit points
module_init(proc_write_init);
module_exit(proc_write_exit);

// Metadata about the module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Proc write demo module");
