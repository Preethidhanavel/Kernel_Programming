#include<linux/kernel.h>
#include<linux/module.h>
MODULE_LICENSE("GPL");

static int hello_init(void)
{
	char c[]="hello world";
	print_hex_dump_bytes("c: ", DUMP_PREFIX_ADDRESS,c,sizeof(c));
	return 0;
}

static void hello_exit(void)
{
	printk("in exit\n");
}

module_init(hello_init);
module_exit(hello_exit);
