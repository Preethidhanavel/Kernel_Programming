#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/version.h>

MODULE_LICENSE("GPL");
static int my_init(void)
{
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,0,0)
		printk(KERN_INFO"kernel version: old one \n");
	#elif LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
		printk(KERN_INFO"kernel version: new one \n");
	#else
		printk(KERN_INFO"kernel version: moderate one\n");
	#endif
	printk("%u\n",LINUX_VERSION_CODE);
	return 0;
}

static void my_exit(void)
{
	printk(KERN_INFO"module removed\n");
}

module_init(my_init);
module_exit(my_exit);
