#include<linux/kernel.h>
#include<linux/module.h>

MODULE_LICENSE("GPL");
static int my_init(void)
{
	printk(KERN_INFO"in %s function\n",__func__);
	return 0;
}

static void my_exit(void)
{
	printk(KERN_INFO"in %s function\n",__func__);
}

module_init(my_init);
module_exit(my_exit);

