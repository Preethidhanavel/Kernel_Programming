#include<linux/kernel.h>
#include<linux/module.h>

MODULE_LICENSE("GPL");
static int fun_init(void)
{
	printk(KERN_DEBUG" in init 1\n");
	pr_debug("%s: In init2\n",__func__);
	return 0;
}

static void fun_exit(void)
{
	pr_debug("%s: in exit2\n",__func__);
	pr_warn("in exit1\n");
}

module_init(fun_init);
module_exit(fun_exit);
