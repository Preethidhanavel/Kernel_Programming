#include <linux/kernel.h>       // Needed for kernel macros like pr_info
#include <linux/module.h>       // Needed for all kernel modules
#include <linux/sched/signal.h> // Needed for for_each_process and task_struct

char buf[256];                  // Buffer for unknown task states

// Function to return the string representation of a task's state
char *get_task_state(long state)
{
    switch (state)
    {
        case TASK_RUNNING:
            return "TASK_RUNNING";          // Task is running
        case TASK_INTERRUPTIBLE:
            return "TASK_INTERRUPTIBLE";    // Task is sleeping and can be woken
        case TASK_UNINTERRUPTIBLE:
            return "TASK_UNINTERRUPTIBLE";  // Task is sleeping and cannot be woken
        case __TASK_STOPPED:
            return "__TASK_STOPPED";        // Task is stopped
        case __TASK_TRACED:
            return "__TASK_TRACED";         // Task is being traced/debugged
        default:
            sprintf(buf, "unknown state:%ld\n", state); // Unknown state
            return buf;
    }
}

// Module initialization function
static int my_init(void)
{
    struct task_struct *task_list;
    unsigned int process_count = 0;

    pr_info("%s:\n", __func__);  // Print module function name

    // Iterate over all processes in the system
    for_each_process(task_list) {
        pr_info("process: %s\t PID:[%d]\t state:%s\n",
                task_list->comm,             // Process name
                task_list->pid,              // Process ID
                get_task_state(task_list->state)); // Process state
        process_count++;
    }

    pr_info("number of processes: %u\n", process_count); // Total process count
    return 0; // Successful initialization
}

// Module cleanup function
static void my_exit(void)
{
    pr_info("%s: in exit\n", __func__); // Inform module exit
}

// Declare GPL license
MODULE_LICENSE("GPL");

// Register init and exit functions
module_init(my_init);
module_exit(my_exit);
