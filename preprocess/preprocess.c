#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICESNSE("GPL");
static int init_oops(void)
{
#ifdef DEBUG
	printk(KERN_INFO"%s: in init\n",__func__);
#endif
	return 0;	
}

static void exit_oops(void)
{
#ifdef DEBUG
	printk(KERN_INFO"%s \n",__func__);
#endif
}

module_init(init_oops);
module_exit(exit_oops);
