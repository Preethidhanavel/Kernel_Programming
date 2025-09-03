#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/sched.h>
#include<asm/current.h>

static int  my_init(void)
{
	printk("current pid: %d, current process: %s\n",current->pid,current->comm);
	return 0;
}

static void my_exit(void)
{
	printk("current pid: %d current process: %s\n",current->pid,current->comm);
}

module_init(my_init);
module_exit(my_exit);
