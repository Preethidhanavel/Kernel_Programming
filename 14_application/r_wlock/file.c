// rwstats.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/rwlock.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/random.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "rwstats"
#define CLASS_NAME  "rwstats_class"

struct my_stats {
    unsigned long packets;
    unsigned long errors;
    unsigned long last_update_jiffies;
};

static struct my_stats stats;
static rwlock_t stats_lock;
static struct timer_list stats_timer;

static dev_t devt;
static struct cdev rw_cdev;
static struct class *rw_class;
static struct device *rw_device;

static unsigned int major_number = 0;

/* ---- char device read ---- */
static ssize_t rwstats_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    char kbuf[128];
    int len;
    unsigned long flags;
    struct my_stats local;

    /* take read lock (disable interrupts while reading) */
    read_lock_irqsave(&stats_lock, flags);
    local = stats; /* copy protected data */
    read_unlock_irqrestore(&stats_lock, flags);

    len = snprintf(kbuf, sizeof(kbuf),
                   "packets: %lu\nerrors: %lu\nlast_update_jiffies: %lu\n",
                   local.packets, local.errors, local.last_update_jiffies);

    return simple_read_from_buffer(buf, count, ppos, kbuf, len);
}

static int rwstats_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    return 0;
}

static int rwstats_release(struct inode *inode, struct file *file)
{
    module_put(THIS_MODULE);
    return 0;
}

static const struct file_operations rwstats_fops = {
    .owner   = THIS_MODULE,
    .read    = rwstats_read,
    .open    = rwstats_open,
    .release = rwstats_release,
};

/* ---- timer callback: simulate writer updating stats ---- */
static void stats_timer_fn(struct timer_list *t)
{
    unsigned long flags;

    /* exclusive write update (disable interrupts while updating) */
    write_lock_irqsave(&stats_lock, flags);
    stats.packets++;
    /* random occasional error */
    if ((prandom_u32() % 10) == 0)
        stats.errors++;
    stats.last_update_jiffies = jiffies;
    write_unlock_irqrestore(&stats_lock, flags);

    /* re-arm timer (1s) */
    mod_timer(&stats_timer, jiffies + msecs_to_jiffies(1000));
}

/* ---- module init/exit ---- */
static int __init rwstats_init(void)
{
    int ret;
    struct device *dev;

    pr_info("rwstats: init\n");

    /* init data & lock */
    stats.packets = 0;
    stats.errors = 0;
    stats.last_update_jiffies = jiffies;
    rwlock_init(&stats_lock);

    /* allocate char device region */
    ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("rwstats: alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }
    major_number = MAJOR(devt);

    cdev_init(&rw_cdev, &rwstats_fops);
    rw_cdev.owner = THIS_MODULE;

    ret = cdev_add(&rw_cdev, devt, 1);
    if (ret) {
        pr_err("rwstats: cdev_add failed: %d\n", ret);
        unregister_chrdev_region(devt, 1);
        return ret;
    }

    rw_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(rw_class)) {
        pr_err("rwstats: class_create failed\n");
        cdev_del(&rw_cdev);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(rw_class);
    }

    dev = device_create(rw_class, NULL, devt, NULL, DEVICE_NAME);
    if (IS_ERR(dev)) {
        pr_err("rwstats: device_create failed\n");
        class_destroy(rw_class);
        cdev_del(&rw_cdev);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(dev);
    }

    /* setup timer */
    timer_setup(&stats_timer, stats_timer_fn, 0);
    mod_timer(&stats_timer, jiffies + msecs_to_jiffies(1000));

    pr_info("rwstats: registered /dev/%s major=%u\n", DEVICE_NAME, major_number);
    return 0;
}

static void __exit rwstats_exit(void)
{
    del_timer_sync(&stats_timer);
    device_destroy(rw_class, devt);
    class_destroy(rw_class);
    cdev_del(&rw_cdev);
    unregister_chrdev_region(devt, 1);

    pr_info("rwstats: exit\n");
}

module_init(rwstats_init);
module_exit(rwstats_exit);

