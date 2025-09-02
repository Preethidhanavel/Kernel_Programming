#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init test_init(void)
{
            printk(KERN_INFO KERN_CONT"%s EMBEDDED",__func__);
	    printk(KERN_INFO KERN_CONT"SYSTEM\n");
    return 0;
}

static void __exit test_exit(void)
{
    printk(KERN_INFO "Exiting module\n");
}

module_init(test_init);
module_exit(test_exit);

