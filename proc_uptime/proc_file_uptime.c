#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>   

#define PROC_NAME "myinfo"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Proc demo with uptime");

static char *author = "Anonymous";
module_param(author, charp, 0000);
MODULE_PARM_DESC(author, "Author name");

static int counter = 0; 

static int show_myinfo(struct seq_file *m, void *v)
{
    unsigned long uptime_sec = jiffies / HZ;  // convert jiffies to seconds

    counter++;  

    seq_printf(m, "Hello from kernel space!\n");
    seq_printf(m, "System uptime: %lu seconds\n", uptime_sec);
    seq_printf(m, "Author: %s\n", author);
    seq_printf(m, "Read counter: %d\n", counter);

    return 0;
}

static int myinfo_open(struct inode *inode, struct file *file)
{
    return single_open(file, show_myinfo, NULL);
}

static const struct file_operations myinfo_fops = {
    .owner   = THIS_MODULE,
    .open    = myinfo_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static int __init myinfo_init(void)
{
    proc_create(PROC_NAME, 0, NULL, &myinfo_fops);
    pr_info("/proc/%s created\n", PROC_NAME);
    return 0;
}

static void __exit myinfo_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("/proc/%s removed\n", PROC_NAME);
}

module_init(myinfo_init);
module_exit(myinfo_exit);

