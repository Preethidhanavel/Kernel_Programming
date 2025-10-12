#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

static int test_value = 10;
static char *author = "LinuxLearner";
static int repeat_count = 1;   // new parameter, default 1

module_param(test_value, int, 0660);
MODULE_PARM_DESC(test_value, "An integer value");

module_param(author, charp, 0660);
MODULE_PARM_DESC(author, "Author's name");

module_param(repeat_count, int, 0660);   // new parameter
MODULE_PARM_DESC(repeat_count, "Number of times to print author");

static int __init param_module_init(void)
{
    int i;
    printk(KERN_INFO "param_module loaded.\n");
    printk(KERN_INFO "test_value = %d\n", test_value);
    printk(KERN_INFO "author = %s\n", author);

    for(i = 0; i < repeat_count; i++)
        printk(KERN_INFO "Author = %s\n", author);

    return 0;
}

static void __exit param_module_exit(void)
{
    printk(KERN_INFO "param_module removed.\n");
}

module_init(param_module_init);
module_exit(param_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Kernel Module with Parameters");

