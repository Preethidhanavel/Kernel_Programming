#include<linux/kernel.h>
#include<linux/module.h>

MODULE_LICENSE("GPL");
char buf[] = "hello";
static int my_init(void)
{
	printk(KERN_INFO"in %s function buffer:%s\n",__func__,buf);
	return 0;
}

static void my_exit(void)
{
	printk(KERN_INFO"exit function\n");
}

module_init(my_init);
module_exit(my_exit);
