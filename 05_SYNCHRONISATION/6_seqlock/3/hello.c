#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seqlock.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

DEFINE_SEQLOCK(mylock);
static struct task_struct *thread1, *thread2, *thread3, *thread4;

int counter = 0;

static int write_threadfn(void *arg)
{
	pr_info("processor:%d trying to acquire write seq lock\n", smp_processor_id());
	write_seqlock(&mylock);
	pr_info("processor:%d acquired write lock\n", smp_processor_id());
	counter++;
	mdelay(7000);
	write_sequnlock(&mylock);
	pr_info("processor%d released write lock\n", smp_processor_id());
	return 0;
}

static int read_threadfn(void *arg)
{
	unsigned int seq;
	int mycounter;
	pr_info("processor:%d starting seqread\n", smp_processor_id());
	do
	{
		seq = read_seqbegin(&mylock);
		mycounter = counter;
		pr_info("processor%d:seq read mycounter:%d\n", smp_processor_id(), mycounter);
	}while (read_seqretry(&mylock, seq));
	pr_info("processor:%d completed seqread\n", smp_processor_id());
	pr_info("processor%d\t counter:%d\n", smp_processor_id(), mycounter);
	return 0;
}

static int __init test_hello_init(void)
{
	thread1 = kthread_run(read_threadfn, NULL, "thread1");
	thread2 = kthread_run(write_threadfn, NULL, "thread2");
	thread3 = kthread_run(read_threadfn, NULL, "thread3");
	thread4 = kthread_run(write_threadfn, NULL, "thread4");
	return 0;
}

static void __exit test_hello_exit(void)
{
}

module_init(test_hello_init);
module_exit(test_hello_exit);
