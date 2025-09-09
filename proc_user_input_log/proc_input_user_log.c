#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define PROC_NAME "writer"
#define MAX_LEN 100

static char message[MAX_LEN];
static int logging_enabled = 0;  // ON/OFF flag

static struct proc_dir_entry *proc_entry;

static ssize_t writer_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char status[50];
    int len;

    len = snprintf(status, sizeof(status), "Logging is %s\n",
                   logging_enabled ? "ON" : "OFF");

    return simple_read_from_buffer(buf, count, ppos, status, len);
}

static ssize_t writer_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    if (count > MAX_LEN - 1)
        count = MAX_LEN - 1;

    if (copy_from_user(message, buf, count))
        return -EFAULT;

    message[count] = '\0';  // Null-terminate

    if (strncmp(message, "ON", 2) == 0)
        logging_enabled = 1;
    else if (strncmp(message, "OFF", 3) == 0)
        logging_enabled = 0;

    printk(KERN_INFO "Received from user: %s, logging_enabled=%d\n",
           message, logging_enabled);

    return count;
}

static const struct file_operations writer_fops = {
    .owner   = THIS_MODULE,
    .read    = writer_read,
    .write   = writer_write,
};

static int __init proc_write_init(void)
{
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &writer_fops);
    if (!proc_entry) {
        pr_err("Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("/proc/%s created. You can write ON/OFF!\n", PROC_NAME);
    return 0;
}

static void __exit proc_write_exit(void)
{
    if (proc_entry)
        remove_proc_entry(PROC_NAME, NULL);

    pr_info("/proc/%s removed.\n", PROC_NAME);
}

module_init(proc_write_init);
module_exit(proc_write_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Proc write ON/OFF demo module");

