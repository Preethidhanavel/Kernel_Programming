#include<linux/kernel.h>
#include<linux/module.h>
#include<generated/utsrelease.h>

MODULE_LICENSE("GPL");
static int my_init(void)
{
	printk(KERN_INFO"init function\t kernel_version:%s\n",UTS_RELEASE);
	return 0;
}

static void my_exit(void)
{
	printk("exit function\n");
}

module_init(my_init);
module_exit(my_exit);
