#include<linux/kernel.h>
#include<linux/module.h>
static int my_init(void)
{
	pr_info("dump stack in init\n");
	dump_stack();
	pr_info("dump stack end\n");
	return 0;
}

static void my_exit(void)
{
	printk("in exit\n");
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
