#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICENSE("GPL");
static int panic_init(void)
{
	printk(KERN_INFO"%s: in init\n",__func__);
	panic("hello kernel - panic attack\n");
	return 0;
}

static void panic_exit(void)
{
	printk(KERN_INFO"%s: in exit\n",__func__);
}

module_init(panic_init);
module_exit(panic_exit);
