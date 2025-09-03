#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>   // for_each_process

MODULE_LICENSE("GPL");

static struct task_struct *t1;

int thread_fun1(void *data)
{
    struct task_struct *task;

    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 1: Listing all tasks\n");

        for_each_process(task) {
            if (task->state == TASK_RUNNING)
                printk(KERN_INFO "PID: %d | Name: %s | State: RUNNING\n",
                       task->pid, task->comm);
        }

        ssleep(3); // sleep to avoid flooding dmesg
    }
    return 0;
}


static int __init my_init(void)
{
    printk(KERN_INFO "Init function: Starting threads\n");

    t1 = kthread_run(thread_fun1, NULL, "thread_1");
    if (IS_ERR(t1)) {
        printk(KERN_ERR "Failed to create thread_1\n");
        t1 = NULL;
    }


    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "Exit function: Stopping threads\n");

    if (t1)
        kthread_stop(t1);
}

module_init(my_init);
module_exit(my_exit);

