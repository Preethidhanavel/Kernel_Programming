#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_NAME "myinfo"

static int show_myinfo(struct seq_file *m, void *v)
{
    seq_printf(m, "Hello from kernel space!\n");
    seq_printf(m, "Module: proc_demo\n");
    seq_printf(m, "Status: Running and happy\n");
    return 0;
}

static int open_myinfo(struct inode *inode, struct file *file)
{
    return single_open(file, show_myinfo, NULL);
}

static const struct file_operations myinfo_fops = {
    .owner   = THIS_MODULE,
    .open    = open_myinfo,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int __init proc_demo_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &myinfo_fops);
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit proc_demo_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Kernel module with /proc entry");

