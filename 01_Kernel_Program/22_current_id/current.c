#include <linux/kernel.h>   // Core kernel functions 
#include <linux/module.h>   // Needed for all kernel modules 
#include <linux/sched.h>    // Contains task_struct definition
#include <asm/current.h>    // Provides "current" pointer 

// Initialization function
static int my_init(void)
{
    // "current" is a pointer to the task_struct of the process
    // current->pid  = process ID
    // current->comm = process name
    printk("current pid: %d, current process: %s\n", current->pid, current->comm);

    return 0;   // Return 0 means module loaded successfully
}

// Cleanup function
static void my_exit(void)
{
    // At removal, "current" points to the process executing rmmod
    printk("current pid: %d, current process: %s\n", current->pid, current->comm);
}

// Register init and exit functions with kernel
module_init(my_init);   // Called when module is inserted
module_exit(my_exit);   // Called when module is removed
