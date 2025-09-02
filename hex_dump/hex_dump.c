#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICENSE("GPL");

static int hello_init(void)
{
	char c[]="hello world";
	print_hex_dump(KERN_ALERT, "c: ", DUMP_PREFIX_ADDRESS,16,1,c,sizeof(c),1);
	return 0;
}

static void hello_exit(void)
{
	printk("in exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
