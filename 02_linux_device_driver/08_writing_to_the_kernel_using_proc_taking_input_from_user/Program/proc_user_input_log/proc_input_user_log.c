#include <linux/init.h>        // For module_init, module_exit
#include <linux/module.h>      // Core header for LKMs
#include <linux/kernel.h>      // For printk and pr_info
#include <linux/proc_fs.h>     // For proc filesystem support
#include <linux/uaccess.h>     // For copy_from_user
#include <linux/string.h>      // For string operations (strncmp)

#define PROC_NAME "writer"     // /proc entry name
#define MAX_LEN 100            // Max size of input buffer

static char message[MAX_LEN];  // Buffer for storing user input
static int logging_enabled = 0; // Flag to track ON/OFF state

static struct proc_dir_entry *proc_entry; // Handle to /proc entry

// -------- READ FUNCTION --------
// Called when user reads /proc/writer 
static ssize_t writer_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char status[50];
    int len;

    // Prepare status string based on logging_enabled flag
    len = snprintf(status, sizeof(status), "Logging is %s\n",
                   logging_enabled ? "ON" : "OFF");

    // Copy status to user space
    return simple_read_from_buffer(buf, count, ppos, status, len);
}

// -------- WRITE FUNCTION --------
// Called when user writes into /proc/writer 
static ssize_t writer_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    // Limit input length
    if (count > MAX_LEN - 1)
        count = MAX_LEN - 1;

    // Copy data from user space into kernel buffer
    if (copy_from_user(message, buf, count))
        return -EFAULT;

    message[count] = '\0';  // Null terminate the input string

    // Compare input and set logging_enabled accordingly
    if (strncmp(message, "ON", 2) == 0)
        logging_enabled = 1;
    else if (strncmp(message, "OFF", 3) == 0)
        logging_enabled = 0;

    // Print to kernel log
    printk(KERN_INFO "Received from user: %s, logging_enabled=%d\n",
           message, logging_enabled);

    return count; // Return number of bytes written
}

// -------- File operations --------
// Defines how /proc/writer behaves
static const struct file_operations writer_fops = {
    .owner   = THIS_MODULE,
    .read    = writer_read,   // Handle reads
    .write   = writer_write,  // Handle writes
};

// -------- Module init function --------
// Called when module is inserted (insmod)
static int __init proc_write_init(void)
{
    // Create /proc/writer with read/write permissions
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &writer_fops);
    if (!proc_entry) {
        pr_err("Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM; // Return error if creation failed
    }

    pr_info("/proc/%s created. You can write ON/OFF!\n", PROC_NAME);
    return 0;
}

// -------- Module exit function --------
// Called when module is removed (rmmod)
static void __exit proc_write_exit(void)
{
    if (proc_entry) // Remove only if it exists
        remove_proc_entry(PROC_NAME, NULL);

    pr_info("/proc/%s removed.\n", PROC_NAME);
}

// Register init and exit functions
module_init(proc_write_init);
module_exit(proc_write_exit);

// -------- Module metadata --------
MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Proc write ON/OFF demo module");
