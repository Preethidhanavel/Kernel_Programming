#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/version.h>

MODULE_LICENSE("GPL");
static int my_init(void)
{
	printk(KERN_INFO"init function\t kernel_version:%u\n",LINUX_VERSION_CODE);
	printk(KERN_INFO"kernel_version 5.4.233 %u\n",KERNEL_VERSION(5,4,233));
	printk(KERN_INFO"kernel_version 4.5.3 %u\n",KERNEL_VERSION(4,5,3));
	return 0;
}

static void my_exit(void)
{
	printk("exit function\n");
}

module_init(my_init);
module_exit(my_exit);
