#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "parking_dev"
#define MAX_SLOTS 8

#define PARK  _IOR('p', 1, int)
#define LEAVE _IOW('p', 2, int)

static dev_t devt;
static struct class *c;
static struct cdev parking_cdev;

static int num_slots = MAX_SLOTS;
static int *slots;                    /* 0 = free, 1 = occupied */
static struct semaphore slots_sem;    /* available slots */
static struct mutex slots_mutex;      /* protects slots[] */
static int major;

static DEFINE_PER_CPU(int, pcpu_alloc_count);

module_param(num_slots, int, 0444);
MODULE_PARM_DESC(num_slots, "Number of parking slots");

static long parking_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int slot, i, user_slot, ret = 0;

    switch (cmd) {
    case PARK:
        if (down_interruptible(&slots_sem))
            return -EINTR;

        if (mutex_lock_interruptible(&slots_mutex)) {
            up(&slots_sem);
            return -EINTR;
        }

        slot = -1;
        for (i = 0; i < num_slots; i++) {
            if (slots[i] == 0) {
                slots[i] = 1;
                slot = i;
                this_cpu_inc(pcpu_alloc_count);
                break;
            }
        }

        mutex_unlock(&slots_mutex);

        if (slot < 0) {
            up(&slots_sem);
            return -EFAULT;
        }

        if (copy_to_user((int __user *)arg, &slot, sizeof(slot)))
            return -EFAULT;

        ret = slot;
        break;

    case LEAVE:
        if (copy_from_user(&user_slot, (int __user *)arg, sizeof(user_slot)))
            return -EFAULT;

        if (user_slot < 0 || user_slot >= num_slots)
            return -EINVAL;

        if (mutex_lock_interruptible(&slots_mutex))
            return -EINTR;

        if (slots[user_slot] == 0) {
            mutex_unlock(&slots_mutex);
            return -EINVAL;
        }

        slots[user_slot] = 0;
        mutex_unlock(&slots_mutex);
        up(&slots_sem);

        {
            int cpu;
            for_each_possible_cpu(cpu)
                pr_info("CPU %d allocated %d slots\n",
                        cpu, per_cpu(pcpu_alloc_count, cpu));
        }

        ret = 0;
        break;

    default:
        ret = -ENOTTY;
    }
    return ret;
}

static int parking_open(struct inode *inode, struct file *file)
{
    pr_info("parking_dev: open\n");
    return 0;
}

static int parking_release(struct inode *inode, struct file *file)
{
    pr_info("parking_dev: release\n");
    return 0;
}

static const struct file_operations parking_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = parking_ioctl,
    .open = parking_open,
    .release = parking_release,
};

static int __init parking_init(void)
{
    int ret, i;

    if (num_slots <= 0 || num_slots > 1024) {
        pr_warn("Invalid num_slots %d, using default %d\n", num_slots, MAX_SLOTS);
        num_slots = MAX_SLOTS;
    }

    ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    major = MAJOR(devt);
    cdev_init(&parking_cdev, &parking_fops);
    parking_cdev.owner = THIS_MODULE;

    ret = cdev_add(&parking_cdev, devt, 1);
    if (ret) {
        unregister_chrdev_region(devt, 1);
        return ret;
    }

    slots = kzalloc(sizeof(int) * num_slots, GFP_KERNEL);
    if (!slots) {
        cdev_del(&parking_cdev);
        unregister_chrdev_region(devt, 1);
        return -ENOMEM;
    }

    c = class_create(THIS_MODULE,"parking_class");
    device_create(c, NULL, devt, NULL, "park_dev");

    sema_init(&slots_sem, num_slots);
    mutex_init(&slots_mutex);

    for (i = 0; i < num_slots; i++)
        slots[i] = 0;

    pr_info("parking_dev: loaded major=%d num_slots=%d\n", major, num_slots);
    return 0;
}

static void __exit parking_exit(void)
{
    device_destroy(c, devt);
    class_destroy(c);
    kfree(slots);
    cdev_del(&parking_cdev);
    unregister_chrdev_region(devt, 1);
    pr_info("parking_dev: unloaded\n");
}

module_init(parking_init);
module_exit(parking_exit);

MODULE_LICENSE("GPL");



