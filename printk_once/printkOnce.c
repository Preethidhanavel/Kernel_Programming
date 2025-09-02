#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

static int __init test_init(void)
{
    int i;
    for (i = 0; i < 20; i++) {
            printk_once(KERN_INFO"Message %d\n", i);
    }
    return 0;
}

static void __exit test_exit(void)
{
    printk(KERN_INFO "Exiting module\n");
}

module_init(test_init);
module_exit(test_exit);

