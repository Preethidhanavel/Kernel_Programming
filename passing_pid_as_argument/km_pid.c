#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/moduleparam.h>
#include<linux/sched.h>
#include<linux/sched/signal.h>

MODULE_LICENSE("GPL");
static unsigned int PID=1;
module_param(PID, uint,0400);
void print_task(struct task_struct *task)
{
	printk("process: %s, parent process: %s\n",task->comm, task->parent->comm);
}

static int task_init(void)
{
	struct task_struct *task = NULL;
	for_each_process(task){
		if(task->pid == (pid_t)PID){
			print_task(task);
		}
	}

	return 0;
}

static void task_exit(void)
{
	printk("good bye!\n");
}

module_init(task_init);
module_exit(task_exit);
