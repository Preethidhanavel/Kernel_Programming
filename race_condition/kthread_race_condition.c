#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

static struct task_struct *t1;
static struct task_struct *t2;

static int shared_counter = 0;   

int thread_fn(void *data)
{
    int i;

    for (i = 0; i < 100000; i++) {
        shared_counter++;
    }

    printk(KERN_INFO "finished thread-1, counter=%d\n",  shared_counter);
    return 0;
}
int thread_fn2(void *data)
{
	int i;
	for(i=0;i<100000;i++)
		shared_counter++;
	printk(KERN_INFO"finished thread-2 counter=%d\n",shared_counter);
	return 0;
}
static int __init race_init(void)
{
    
    printk(KERN_INFO "Race condition module loaded\n");

    t1 = kthread_run(thread_fn, NULL, "thread_1");
    if (IS_ERR(t1)) {
        printk(KERN_ERR "Failed to create thread_1\n");
        t1 = NULL;
    }

    t2 = kthread_run(thread_fn,NULL, "thread_2");
    if (IS_ERR(t2)) {
        printk(KERN_ERR "Failed to create thread_2\n");
        t2=NULL;
    }

    return 0;
}

static void __exit race_exit(void)
{
    if (t1)
        kthread_stop(t1);
    if (t2)
        kthread_stop(t2);

    printk(KERN_INFO "Race condition module unloaded. Final counter=%d\n", shared_counter);
}

module_init(race_init);
module_exit(race_exit);

