#include <linux/kernel.h>   // Needed for kernel macros like printk
#include <linux/module.h>   // Needed for all kernel modules
#include <linux/kthread.h>  // Needed for kernel threads
#include <linux/delay.h>    // Needed for delays (if used)

MODULE_LICENSE("GPL");      // Declare the license type

// Pointers to hold kernel thread structures
static struct task_struct *t1;
static struct task_struct *t2;

// Shared counter variable (race condition will occur)
static int shared_counter = 0;   

// First kernel thread function
int thread_fn(void *data)
{
    int i;

    // Increment shared_counter 100,000 times
    for (i = 0; i < 100000; i++) {
        shared_counter++;
    }

    printk(KERN_INFO "finished thread-1, counter=%d\n",  shared_counter);
    return 0;
}

// Second kernel thread function
int thread_fn2(void *data)
{
    int i;

    // Increment shared_counter 100,000 times
    for (i = 0; i < 100000; i++)
        shared_counter++;

    printk(KERN_INFO "finished thread-2, counter=%d\n", shared_counter);
    return 0;
}

// Module initialization function
static int __init race_init(void)
{
    printk(KERN_INFO "Race condition module loaded\n");

    // Start first kernel thread
    t1 = kthread_run(thread_fn, NULL, "thread_1");
    if (IS_ERR(t1)) {   // Check for errors
        printk(KERN_ERR "Failed to create thread_1\n");
        t1 = NULL;
    }

    // Start second kernel thread
    t2 = kthread_run(thread_fn2, NULL, "thread_2");
    if (IS_ERR(t2)) {   // Check for errors
        printk(KERN_ERR "Failed to create thread_2\n");
        t2 = NULL;
    }

    return 0;  // Successful initialization
}

// Module cleanup function
static void __exit race_exit(void)
{
    // Stop kernel threads if they were created
    if (t1)
        kthread_stop(t1);
    if (t2)
        kthread_stop(t2);

    // Print the final value of the shared counter
    printk(KERN_INFO "Race condition module unloaded. Final counter=%d\n", shared_counter);
}

// Register init and exit functions
module_init(race_init);
module_exit(race_exit);
