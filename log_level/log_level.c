#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICENSE("GPL");

static int fun_init(void)
{
	printk("\001""4""in init\n");
	return 0;
}

static void fun_exit(void)
{
	printk("\001""6""in exit\n");
}

module_init(fun_init);
module_exit(fun_exit);
