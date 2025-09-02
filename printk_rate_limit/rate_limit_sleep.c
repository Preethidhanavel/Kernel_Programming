#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ratelimit.h>
#include<linux/delay.h>
MODULE_LICENSE("GPL");

static int __init test_init(void)
{
    int i;
    for (i = 0; i < 50; i++) {
        if (printk_ratelimit())
            printk(KERN_INFO "Message %d\n", i);
	else
	{
		pr_info("sleeping for 5 seconds\n");
		msleep(5000);
	}
    }
    return 0;
}

static void __exit test_exit(void)
{
    printk(KERN_INFO "Exiting module\n");
}

module_init(test_init);
module_exit(test_exit);

