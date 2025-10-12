#include <linux/init.h>      // For module init/exit macros
#include <linux/module.h>    // For all kernel modules
#include <linux/kernel.h>    // For printk()
#include <linux/list.h>      // For linked list support
#include <linux/slab.h>      // For kmalloc() and kfree()

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Kernel Linked List Example");

// Define a student structure with an id, name, and list_head
struct student {
    int id;
    char name[20];
    struct list_head list;   // Kernel's linked list pointer
};

// Declare and initialize the head of the linked list
static LIST_HEAD(student_list);

// Module initialization function
static int __init linkedlist_init(void)
{
    struct student *stud;
    int i;

    printk(KERN_INFO "Linked list example: init\n");

    // Create 5 student nodes and add them to the list
    for (i = 1; i <= 5; i++) {
        stud = kmalloc(sizeof(*stud), GFP_KERNEL);  // Allocate memory
        if (!stud)
            return -ENOMEM;  // If memory allocation fails, exit

        stud->id = i;
        snprintf(stud->name, sizeof(stud->name), "Student%d", i);

        // Add new node at the end of the list
        list_add_tail(&stud->list, &student_list);
    }

    // Traverse and print the student list
    printk(KERN_INFO "Traversing student list:\n");
    list_for_each_entry(stud, &student_list, list) {
        printk(KERN_INFO "ID=%d Name=%s\n", stud->id, stud->name);
    }

    return 0;
}

// Module exit function
static void __exit linkedlist_exit(void)
{
    struct student *stud, *tmp;

    printk(KERN_INFO "Linked list example: exit\n");

    // Safe traversal: delete nodes while iterating
    list_for_each_entry_safe(stud, tmp, &student_list, list) {
        printk(KERN_INFO "Freeing student ID=%d Name=%s\n", stud->id, stud->name);
        list_del(&stud->list);   // Remove node from list
        kfree(stud);             // Free allocated memory
    }
}

// Register init and exit functions
module_init(linkedlist_init);
module_exit(linkedlist_exit);
