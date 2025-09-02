#include<linux/kernel.h>
#include<linux/module.h>

static int init_oops(void)
{
	printk(KERN_INFO"%s: in init\n",__func__);
	*(int *)0X12 = 'a';
	return 0;
}

static void exit_oops(void)
{
	printk(KERN_INFO"%s \n",__func__);
}

module_init(init_oops);
module_exit(exit_oops);
