#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

static struct task_struct *t1;
static struct task_struct *t2;

int thread_fun1(void *data)
{
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 1 running...\n");
        ssleep(1); // sleep for 1 sec
    }
    return 0;
}

int thread_fun2(void *data)
{
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 2 running...\n");
        ssleep(2); // sleep for 2 sec
    }
    return 0;
}

static int __init my_init(void)
{
    printk(KERN_INFO "Init function\n");

    t1 = kthread_create(thread_fun1, NULL, "thread_1");
    if (!IS_ERR(t1))
        wake_up_process(t1);

    t2 = kthread_create(thread_fun2, NULL, "thread_2");
    if (!IS_ERR(t2))
        wake_up_process(t2);

    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "Exit function\n");
    if (t1)
        kthread_stop(t1);
    if (t2)
        kthread_stop(t2);
}

module_init(my_init);
module_exit(my_exit);

