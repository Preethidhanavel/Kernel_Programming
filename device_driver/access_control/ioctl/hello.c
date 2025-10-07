#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/sched.h>
#include "ioctl_cmd.h"


int base_minor = 0;
char *device_name = "sig";
int count = 1;
dev_t devicenumber;

static struct class *class = NULL;
static struct device *device = NULL;
static struct cdev mycdev;

static int sig_pid = 0;
static struct task_struct *sig_tsk = NULL;
static int sig_tosend = SIGKILL;

#define MAX_SIZE        1024
MODULE_LICENSE("GPL");


static int device_open(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	pr_info("%s\n", __func__);
        return 0;
}


long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned char ch;
	int retval = 0;
	long size = _IOC_SIZE(cmd);

	pr_info("%s: Cmd:%u\t Arg:%lu Size:%lu add:%p\n", __func__, cmd, arg, size, &ch);

	if (_IOC_TYPE(cmd) != SIG_MAGIC_NUMBER) return -ENOTTY;
	if (_IOC_NR(cmd) > SIG_IOCTL_MAX_CMDS) return -ENOTTY;

	//access_ok is kernel-oriented, so the concept of read and write is reversed
	
	retval = access_ok((void __user *)arg, size);

	pr_info("access_ok returned:%d\n", retval);
	if (!retval)
		return -EFAULT;

	switch(cmd)
	{
		//Get Length of buffer
		case SIG_IOCTL_SET_PID:
			pr_info("SIG_IOCTL_SET_PID\n");
			get_user(sig_pid, (unsigned int *)arg);
			pr_info("PID:%d\n", sig_pid);
			sig_tsk = pid_task(find_vpid(sig_pid), PIDTYPE_PID);
			break;
		//clear buffer
		case SIG_IOCTL_SET_SIGNAL:
			pr_info("SIG_IOCTL_SET_SIGNAL\n");
			get_user(sig_tosend, (unsigned int *)arg);
			pr_info("signal:%d\n", sig_tosend);
			break;
		//fill character
		case SIG_IOCTL_SEND_SIGNAL:
			pr_info("SIG_IOCTL_SEND_SIGNAL\n");
			if (!capable(CAP_SYS_ADMIN))
				return -EPERM;
			if (!sig_tsk) {
                        pr_info("You haven't set the pid; using current\n");
                        	sig_tsk = current;
                        	sig_pid = (int)current->pid;
                	}
			retval = send_sig(sig_tosend, sig_tsk, 0);
			pr_info("retval = %d\n", retval);
			break;
		default:
			pr_info("Unknown Command:%u\n", cmd);
			return -ENOTTY;
	}
	return 0;
}





struct file_operations device_fops = {
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl,
};


static int test_hello_init(void)
{
	class = class_create(THIS_MODULE, "myclass");

	if (!alloc_chrdev_region(&devicenumber, base_minor, count, device_name)) {
		printk("Device number registered\n");
		printk("Major number received:%d\n", MAJOR(devicenumber));

		device = device_create(class, NULL, devicenumber, NULL, device_name);
		cdev_init(&mycdev, &device_fops);
		mycdev.owner = THIS_MODULE;
		cdev_add(&mycdev, devicenumber, count);

	}
	else
		printk("Device number registration Failed\n");

	return 0;
}

static void test_hello_exit(void)
{
	device_destroy(class, devicenumber);
        class_destroy(class);
	cdev_del(&mycdev);
	unregister_chrdev_region(devicenumber, count);
}

module_init(test_hello_init);
module_exit(test_hello_exit);
