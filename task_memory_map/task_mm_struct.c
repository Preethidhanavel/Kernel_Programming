#include <linux/kernel.h>        // Needed for printk and kernel macros
#include <linux/module.h>        // Needed for all kernel modules
#include <linux/moduleparam.h>   // Needed for module parameters
#include <linux/list.h>          // Needed for linked list operations
#include <linux/mm.h>            // Needed for mm_struct, vm_area_struct
#include <linux/mm_types.h>      // Additional memory type definitions
#include <linux/proc_fs.h>       // For proc filesystem access 
#include <linux/version.h>       // Kernel version macros
#include <linux/sched.h>         // Needed for task_struct
#include <linux/sched/signal.h>  // Needed for for_each_process

// Function prototypes
static void print_vmarea_mode(struct vm_area_struct *vmnode);
static void print_vm_list(struct vm_area_struct *vmlist);

// Module parameter: PID of the process to inspect
static unsigned int PID = 1;
module_param(PID, uint, 0400);  // Parameter type: unsigned int, permission: read-only

// Helper function to get dentry from a file
struct dentry *file(struct file *pfile)
{
    return pfile->f_path.dentry;
}

// Function to print the file mapped in a VM area
void print_vm_file(struct vm_area_struct *vmarea)
{
    struct file *vmfile = NULL;
    struct dentry *den = NULL;
    struct dentry *pPath = NULL;
    char file[255];
    char *start = NULL, *end = NULL;

    if (vmarea->vm_file == NULL) {
        printk("\t it's not a file map"); // No file mapped
    } else {
        vmfile = vmarea->vm_file;
        den = vmfile->f_path.dentry; // Get dentry

        if (den != NULL) {
            printk(KERN_CONT "\t");
            memset(file, '\0', sizeof(file));

            // Build the full path by traversing parent dentries
            for (pPath = den; pPath != NULL; pPath = pPath->d_parent) {
                if (strcmp(pPath->d_name.name, "/") != 0) {
                    strcpy(file + strlen(file), pPath->d_name.name);
                    strcpy(file + strlen(file), "/");
                    continue;
                } else {
                    break;
                }
            }

            // Print path components in reverse order
            do {
                end = file + strlen(file) - 1;
                for (start = end - 1; *start != '/' && start > file; start--);
                if (*start == '/')
                    start++;
                *end = '\0';
                printk(KERN_CONT "/%s", start);
                *start = '\0';
            } while (start > file);
        }

        printk("\n"); // End line for file output
    }
}

// Function to print VM area flags and addresses
void print_vmarea_mode(struct vm_area_struct *vmnode)
{
    printk("0X%lx\t0x%lx\n", vmnode->vm_start, vmnode->vm_end);

    // Print read/write/execute/shared flags
    printk(KERN_CONT "%c", (vmnode->vm_flags & VM_READ) ? 'r' : '-');
    printk(KERN_CONT "%c", (vmnode->vm_flags & VM_WRITE) ? 'w' : '-');
    printk(KERN_CONT "%c", (vmnode->vm_flags & VM_EXEC) ? 'x' : '-');
    printk(KERN_CONT "%c", (vmnode->vm_flags & VM_SHARED) ? 's' : 'p');

    // Print mapped file if exists
    print_vm_file(vmnode);
}

// Function to print all VM areas in a process
static void print_vm_list(struct vm_area_struct *vmlist)
{
    struct vm_area_struct *vmnode = vmlist;
    while (vmnode != NULL) {
        print_vmarea_mode(vmnode);  // Print one VM area
        vmnode = vmnode->vm_next;   // Move to next VM area
    }
}

// Function to print memory layout from mm_struct
void print_mm_struct(struct mm_struct *mm)
{
    printk("code: 0x%lx, 0x%lx\n", mm->start_code, mm->end_code);
    printk("data: 0x%lx, 0x%lx\n", mm->start_data, mm->end_data);
    printk("heap: 0x%lx, 0x%lx\n", mm->start_brk, mm->brk);
    printk("stack: 0x%lx\n", mm->start_stack);

    // Print VM area list
    print_vm_list(mm->mmap);
}

// Module initialization function
static int init_find(void)
{
    struct task_struct *task = NULL;

    // Iterate over all processes
    for_each_process(task) {
        if (task->pid == (pid_t)PID) {
            print_mm_struct(task->mm); // Print memory info for matching PID
        }
    }
    return 0;
}

// Module exit function
static void exit_find(void)
{
    printk("good bye\n"); // Inform module exit
}

// Register init and exit functions
module_init(init_find);
module_exit(exit_find);

MODULE_LICENSE("GPL"); // Declare GPL license
