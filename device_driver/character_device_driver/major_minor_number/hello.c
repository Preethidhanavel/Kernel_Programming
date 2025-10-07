#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>

MODULE_LICENSE("GPL");
static int test_hello_init(void)
{
	dev_t devicenumber;

	printk("Major Number :%d\n", MAJOR(devicenumber));
	printk("Minor Number :%d\n", MINOR(devicenumber));

	devicenumber = 49;
	printk("Major Number :%d\n", MAJOR(devicenumber));
	printk("Minor Number :%d\n", MINOR(devicenumber));

	devicenumber = MKDEV(120, 30);
	printk("Major Number :%d\n", MAJOR(devicenumber));
	printk("Minor Number :%d\n", MINOR(devicenumber));

	return 0;
}

static void test_hello_exit(void)
{

}

module_init(test_hello_init);
module_exit(test_hello_exit);
