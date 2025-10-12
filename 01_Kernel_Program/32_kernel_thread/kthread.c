#include <linux/kernel.h>    // Needed for kernel functions like printk
#include <linux/module.h>    // Needed for all modules
#include <linux/kthread.h>   // For creating and managing kernel threads
#include <linux/delay.h>     // For ssleep() function

MODULE_LICENSE("GPL");       // Declare license to avoid tainting the kernel

// Declare pointers for two kernel threads
static struct task_struct *t1;
static struct task_struct *t2;

// Function executed by first kernel thread
int thread_fun1(void *data)
{
    // Run until the thread is asked to stop
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 1 running...\n");
        ssleep(1);  // Sleep for 1 second
    }
    return 0;
}

// Function executed by second kernel thread
int thread_fun2(void *data)
{
    // Run until the thread is asked to stop
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 2 running...\n");
        ssleep(2);  // Sleep for 2 seconds
    }
    return 0;
}

// Module initialization function
static int __init my_init(void)
{
    printk(KERN_INFO "Init function\n");

    // Create first thread
    t1 = kthread_create(thread_fun1, NULL, "thread_1");
    if (!IS_ERR(t1))         // Check if thread creation succeeded
        wake_up_process(t1); // Start the thread

    // Create second thread
    t2 = kthread_create(thread_fun2, NULL, "thread_2");
    if (!IS_ERR(t2))         // Check if thread creation succeeded
        wake_up_process(t2); // Start the thread

    return 0;
}

// Module cleanup function
static void __exit my_exit(void)
{
    printk(KERN_INFO "Exit function\n");

    // Stop both threads safely
    if (t1)
        kthread_stop(t1);
    if (t2)
        kthread_stop(t2);
}

// Register module init and exit functions
module_init(my_init);
module_exit(my_exit);
