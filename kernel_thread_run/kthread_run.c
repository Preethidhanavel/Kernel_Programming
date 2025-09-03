#include <linux/kernel.h>        // Needed for printk()
#include <linux/module.h>        // Needed for all modules
#include <linux/kthread.h>       // For kernel threads
#include <linux/delay.h>         // For ssleep()
#include <linux/sched/signal.h>  // For for_each_process() macro

MODULE_LICENSE("GPL");           // Specify license type 

// Pointer to hold our kernel thread structure
static struct task_struct *t1;

// Thread function: runs in a loop until stopped
int thread_fun1(void *data)
{
    struct task_struct *task;

    // Loop until the thread is explicitly stopped
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 1: Listing all tasks\n");

        // Iterate over all processes in the system
        for_each_process(task) {
            // Print details only for tasks that are RUNNING
            if (task->state == TASK_RUNNING)
                printk(KERN_INFO "PID: %d | Name: %s | State: RUNNING\n",
                       task->pid, task->comm);
        }

        // Sleep for 3 seconds to avoid spamming the kernel log
        ssleep(3);
    }
    return 0;
}

// Module initialization function
static int __init my_init(void)
{
    printk(KERN_INFO "Init function: Starting threads\n");

    // Create and run a kernel thread
    t1 = kthread_run(thread_fun1, NULL, "thread_1");

    // Error handling: check if thread creation failed
    if (IS_ERR(t1)) {
        printk(KERN_ERR "Failed to create thread_1\n");
        t1 = NULL;
    }

    return 0;
}

// Module cleanup function
static void __exit my_exit(void)
{
    printk(KERN_INFO "Exit function: Stopping threads\n");

    // Stop the thread if it was successfully created
    if (t1)
        kthread_stop(t1);
}

// Register module init and exit functions
module_init(my_init);
module_exit(my_exit);
