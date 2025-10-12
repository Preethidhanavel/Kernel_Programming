#include <linux/init.h>       // For module init/exit macros
#include <linux/module.h>     // Core header for modules
#include <linux/kernel.h>     // For printk()
#include <linux/proc_fs.h>    // For proc filesystem support
#include <linux/seq_file.h>   // For seq_file operations

#define PROC_NAME "myinfo"    // Name of the /proc entry

// Function that prints information to the /proc file
static int show_myinfo(struct seq_file *m, void *v)
{
    seq_printf(m, "Hello from kernel space!\n");
    seq_printf(m, "Module: proc_demo\n");
    seq_printf(m, "Status: Running and happy\n");
    return 0;
}

// Called when /proc/myinfo is opened
static int open_myinfo(struct inode *inode, struct file *file)
{
    // single_open() helps link our show_myinfo function to /proc file
    return single_open(file, show_myinfo, NULL);
}

// File operations structure for our /proc entry
static const struct file_operations myinfo_fops = {
    .owner   = THIS_MODULE,       // Owner module
    .open    = open_myinfo,       // What to do on open()
    .read    = seq_read,          // How to read data
    .llseek  = seq_lseek,         // Support for seeking
    .release = single_release,    // Cleanup on close
};

// Module init function (runs when module is loaded)
static int __init proc_demo_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &myinfo_fops);   // Create /proc/myinfo
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

// Module exit function (runs when module is removed)
static void __exit proc_demo_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);   // Remove /proc/myinfo
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

// Register init and exit functions
module_init(proc_demo_init);
module_exit(proc_demo_exit);

// Module metadata
MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Kernel module with /proc entry");
