#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICENSE("GPL");
void fun(void)
{
	int i;
	for(i=0;i<50000L;i++)
		printk(KERN_INFO"%d\n",i);
}
static int fun_init(void)
{
	printk(KERN_INFO"in init\n");
	fun();
	return 0;
}

static void fun_exit(void)
{
	printk(KERN_INFO"in exit\n");
}

module_init(fun_init);
module_exit(fun_exit);
