#include <linux/kernel.h>       // Needed for printk()
#include <linux/module.h>       // Needed for all modules
#include <linux/moduleparam.h>  // Needed for module parameters
#include <linux/sched.h>        // Needed for task_struct
#include <linux/sched/signal.h> // Needed for for_each_process macro

MODULE_LICENSE("GPL");          // Specify license type 

// Define a module parameter 'PID' of type unsigned int, default value = 1
// Permissions 0400: readable by root only
static unsigned int PID = 1;
module_param(PID, uint, 0400);

// Function to print task details
void print_task(struct task_struct *task)
{
    printk("process: %s, parent process: %s\n",
           task->comm, task->parent->comm); // Print process name and its parent's name
}

// Module initialization function
static int task_init(void)
{
    struct task_struct *task = NULL;

    // Iterate over all processes in the system
    for_each_process(task) {
        // If the process ID matches the module parameter PID
        if (task->pid == (pid_t)PID) {
            print_task(task); // Print task info
        }
    }

    return 0; // Successful initialization
}

// Module cleanup function
static void task_exit(void)
{
    printk("good bye!\n"); // Print message when module is removed
}

// Register module initialization and cleanup functions
module_init(task_init);
module_exit(task_exit);
