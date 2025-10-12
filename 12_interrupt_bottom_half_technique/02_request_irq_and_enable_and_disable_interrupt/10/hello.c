#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irqflags.h>
#include <linux/interrupt.h>

unsigned int irq = 19;

module_param(irq, int, 0);

static int __init my_init(void)
{
	pr_info("module is loaded on processor:%d\n", smp_processor_id());
	pr_info("Disabling Interrupt:%u\n", irq);
	disable_irq(irq);
	pr_info("Disabled Interrupt:%u\n", irq);
	mdelay(10000L);
	pr_info("Enabling Interrupt:%u\n", irq);
	enable_irq(irq);
	pr_info("Enabled Interrupt:%u\n", irq);
	return 0;
}

static void __exit my_exit(void)
{
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
