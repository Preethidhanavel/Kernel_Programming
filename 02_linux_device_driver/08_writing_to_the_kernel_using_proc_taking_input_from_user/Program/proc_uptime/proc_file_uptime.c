#include <linux/init.h>        // For module_init() and module_exit()
#include <linux/module.h>      // Core header for all modules
#include <linux/kernel.h>      // For printk/pr_info
#include <linux/proc_fs.h>     // For proc filesystem support
#include <linux/seq_file.h>    // For seq_file interface
#include <linux/jiffies.h>     // For jiffies (system ticks)

#define PROC_NAME "myinfo"     // Name of the proc entry (/proc/myinfo)

MODULE_LICENSE("GPL");         // License type
MODULE_AUTHOR("PREETHI");      // Module author
MODULE_DESCRIPTION("Proc demo with uptime");  // Description

// -------- Module parameter --------
static char *author = "Anonymous";     // Default author name
module_param(author, charp, 0000);     // Allow user to pass author name while loading
MODULE_PARM_DESC(author, "Author name");

static int counter = 0;   // Counts how many times file has been read

// -------- Show function --------
// This function prints information when /proc/myinfo is read
static int show_myinfo(struct seq_file *m, void *v)
{
    unsigned long uptime_sec = jiffies / HZ;  // Convert jiffies into seconds

    counter++;   // Increment counter each time file is read

    // Print formatted information into proc file
    seq_printf(m, "Hello from kernel space!\n");
    seq_printf(m, "System uptime: %lu seconds\n", uptime_sec);
    seq_printf(m, "Author: %s\n", author);
    seq_printf(m, "Read counter: %d\n", counter);

    return 0;
}

// -------- Open function --------
// Called when /proc/myinfo is opened
static int myinfo_open(struct inode *inode, struct file *file)
{
    return single_open(file, show_myinfo, NULL);
}

// -------- File operations --------
// Defines how our /proc file behaves
static const struct file_operations myinfo_fops = {
    .owner   = THIS_MODULE,      // Module owner
    .open    = myinfo_open,      // Open handler
    .read    = seq_read,         // Read handler
    .llseek  = seq_lseek,        // Seek handler
    .release = single_release,   // Release handler
};

// -------- Module init function --------
// Runs when module is loaded (insmod)
static int __init myinfo_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &myinfo_fops);   // Create /proc/myinfo
    pr_info("/proc/%s created\n", PROC_NAME);
    return 0;
}

// -------- Module exit function --------
// Runs when module is removed (rmmod)
static void __exit myinfo_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);   // Remove /proc/myinfo
    pr_info("/proc/%s removed\n", PROC_NAME);
}

// Register init and exit functions
module_init(myinfo_init);
module_exit(myinfo_exit);
