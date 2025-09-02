#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICENSE("GPL");

char *name=__stringify(KBUILD_MODNAME);
static int hello_init(void)
{
	printk("%s  \n",__func__);
	printk("modulename:%s\n",name);
	return 0;
}

static void hello_exit(void)
{
	printk("in exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
