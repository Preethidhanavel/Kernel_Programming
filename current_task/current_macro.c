#include <linux/kernel.h>        // Core kernel functions 
#include <linux/module.h>        // Needed for all kernel modules 
#include <linux/sched/signal.h>  // For task_struct and process traversal

// Buffer for storing unknown state messages
char buf[256];

// Helper function to convert task state (numeric) into human-readable string
char * get_task_state(long state)
{
    switch (state)
    {
        case TASK_RUNNING:
            return "TASK_RUNNING";         // Process is running or ready to run
        case TASK_INTERRUPTIBLE:
            return "TASK_INTERRUPTIBLE";   // Process is waiting and can be interrupted
        case TASK_UNINTERRUPTIBLE:
            return "TASK_UNINTERRUPTIBLE"; // Process is waiting and cannot be interrupted
        case __TASK_STOPPED:
            return "__TASK_STOPPED";       // Process has been stopped
        case __TASK_TRACED:
            return "__TASK_TRACED";        // Process is being traced by a debugger
        default:
        {
            // If state does not match known states, return as "unknown"
            sprintf(buf, "unknown state:%ld\n", state);
            return buf;
        }
    }
}

// Initialization function
static int my_init(void)
{
    struct task_struct *task;       // Represents a process
    unsigned int process_count = 0; // Counter for processes

    pr_info("%s:\n", __func__);     // Print function name 

    // Traverse from the current process up to the init_task 
    for (task = current; task != &init_task; task = task->parent) {
        // Print process name, PID, and state
        pr_info("process: %s\t  PID:[%d]\t state:%s\n",
                task->comm, task->pid, get_task_state(task->state));
        process_count++;
    }

    // Print how many processes were traversed
    pr_info("number of processes: %u\n", process_count);

    return 0;   // Successful load
}

// Cleanup function: runs when module is removed 
static void my_exit(void)
{
    pr_info("%s: in exit\n", __func__);
}

// License declaration 
MODULE_LICENSE("GPL");

// Register init and exit functions
module_init(my_init);
module_exit(my_exit);
