#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/cpumask.h>

static int fun_init(void)
{
	pr_info("number of CPU %d\n",num_online_cpus());
	return 0;
}

static void fun_exit(void)
{
	printk("exit function\n");
}

module_init(fun_init);
module_exit(fun_exit);
MODULE_LICENSE("GPL");
