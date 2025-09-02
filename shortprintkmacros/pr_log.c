#include<linux/kernel.h>
#include<linux/module.h>

MODULE_LICENSE("GPL");
static int fun_init(void)
{
	pr_info("%s: In init\n",__func__);
	return 0;
}

static void fun_exit(void)
{
	pr_warn("%s: in exit\n",__func__);
}

module_init(fun_init);
module_exit(fun_exit);
