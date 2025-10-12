#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/slab.h>


#define SHARED_IRQ 12
static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;
module_param(irq, int, S_IRUGO);


void *alloc_mem(unsigned int size)
{
	if (in_interrupt()) {
		return kmalloc(size, GFP_ATOMIC);
	} else {
		return kmalloc(size, GFP_KERNEL);
	}

}
static irqreturn_t my_interrupt(int irq, void *dev_id)
{
	void *mem = alloc_mem(1024);
	kfree(mem);
	return IRQ_NONE;	/* we return IRQ_NONE because we are just observing */
}

static int __init my_init(void)
{
	void *mem = alloc_mem(1024);
	kfree(mem);
	if (request_irq
	    (irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id)) {
		pr_info("Failed to reserve irq %d\n", irq);
		return -1;
	}
	pr_info("Successfully loading ISR handler\n");
	return 0;
}

static void __exit my_exit(void)
{
	synchronize_irq(irq);
	free_irq(irq, &my_dev_id);
	pr_info("Successfully unloading,  irq_counter = %d\n", irq_counter);
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
