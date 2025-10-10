#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/kthread.h>

MODULE_LICENSE("GPL");
DEFINE_MUTEX(my_lock);
struct task_struct *mythread;

static int threadfn(void *data)
{
	pr_info("Kernel thread running on processor:%d\n", smp_processor_id());
	pr_info("Kernel thread unlocking without calling lock on processor:%d\n", smp_processor_id());
	mutex_unlock(&my_lock);
	pr_info("Kernel thread unlocked without calling lock on processor:%d\n", smp_processor_id());
	return 0;
}

static int __init test_hello_init(void)
{
    pr_info("Init function running on processor:%d\n", smp_processor_id());
    mutex_lock_interruptible(&my_lock);
    pr_info("Init function locked on processor:%d\n", smp_processor_id());
    mythread = kthread_run(threadfn, NULL, "mythread");
    msleep(8000);
    pr_info("Init function completed on processor:%d\n", smp_processor_id());
    return -1;
}

static void __exit test_hello_exit(void)
{
}

module_init(test_hello_init);
module_exit(test_hello_exit);
