#include <linux/kernel.h>    // Needed for kernel functions like printk
#include <linux/module.h>    // Needed for module macros
#include <linux/kthread.h>   // For kernel threads
#include <linux/delay.h>     // For ssleep()

MODULE_LICENSE("GPL");       // Specify license type 

// Pointers for two kernel threads
static struct task_struct *t1;
static struct task_struct *t2;

// Function executed by first kernel thread
int thread_fun1(void *data)
{
    int i = 0;

    // Keep running until thread is asked to stop
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 1 running...\n");  // Log thread activity
        printk("thread_1: %d\n", i);               // Print loop counter
        i++;
        ssleep(1);                                  // Sleep for 1 second
    }
    return 0;
}

// Function executed by second kernel thread
int thread_fun2(void *data)
{
    int i = 0;

    // Keep running until thread is asked to stop
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 2 running...\n");  // Log thread activity
        printk("thread_2: %d\n", i);               // Print loop counter
        i++;
        ssleep(2);                                  // Sleep for 2 seconds
    }
    return 0;
}

// Module initialization function
static int __init my_init(void)
{
    printk(KERN_INFO "Init function\n");

    // Create and start first kernel thread
    t1 = kthread_create(thread_fun1, NULL, "thread_1");
    if (!IS_ERR(t1))
        wake_up_process(t1);  // Start the thread if creation succeeded

    // Create and start second kernel thread
    t2 = kthread_create(thread_fun2, NULL, "thread_2");
    if (!IS_ERR(t2))
        wake_up_process(t2);  // Start the thread if creation succeeded

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

// Register initialization and cleanup functions
module_init(my_init);
module_exit(my_exit);
