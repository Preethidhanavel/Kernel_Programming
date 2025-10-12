#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define SHARED_IRQ 19
static int irq = SHARED_IRQ;
module_param(irq, int, S_IRUGO);


static int __init my_init(void)
{
	enable_irq(irq);
	return -1;
}

static void __exit my_exit(void)
{
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
