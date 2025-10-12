#include <linux/kernel.h>       // Needed for printk and kernel macros
#include <linux/module.h>       // Needed for all kernel modules
#include <linux/kthread.h>      // Needed for kernel threads
#include <linux/delay.h>        // Needed for ssleep()
#include <linux/sched/signal.h> // Needed for for_each_process (if used)

MODULE_LICENSE("GPL");          // Declare the license type

// Pointer to hold the kernel thread structure
static struct task_struct *t1;

// Function executed by the kernel thread
int thread_fun1(void *data)
{
    // Run until kthread_should_stop() returns true
    while (!kthread_should_stop()) {
        printk(KERN_INFO "Thread 1: running\n");              // Print message from thread
        printk("Processor ID from thread 1: %d\n", smp_processor_id()); // Print CPU ID
        ssleep(3);                                           // Sleep 3 seconds to avoid flooding dmesg
    }
    return 0;
}

// Module initialization function
static int __init my_init(void)
{
    printk(KERN_INFO "Init function: Starting threads\n");  // Inform module init

    printk("Processor ID from init function: %d\n", smp_processor_id()); // Print CPU ID

    // Create and start the kernel thread
    t1 = kthread_run(thread_fun1, NULL, "thread_1");
    if (IS_ERR(t1)) {                                       // Check for errors
        printk(KERN_ERR "Failed to create thread_1\n");
        t1 = NULL;
    }

    return 0;  // Successful initialization
}

// Module cleanup function
static void __exit my_exit(void)
{
    printk(KERN_INFO "Exit function: Stopping threads\n");  // Inform module exit

    printk("Processor ID from exit function: %d\n", smp_processor_id()); // Print CPU ID

    // Stop the kernel thread if it was created
    if (t1)
        kthread_stop(t1);
}

// Register init and exit functions
module_init(my_init);
module_exit(my_exit);
