#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/sched/signal.h>

char buf[256];

char * get_task_state(long state)
{
	switch(state)
	{
		case TASK_RUNNING:
			return "TASK_RUNNING";
		case TASK_INTERRUPTIBLE:
			return "TASK_INTERRUPIBLE";
		case TASK_UNINTERRUPTIBLE:
			return "TASK_UNINTERRUPTIBLE";
		case __TASK_STOPPED:
			return "__TASK_STOPPED";
		case __TASK_TRACED:
			return "__TASK_TRACED";
		default:
			{
				sprintf(buf,"unknown state:%ld\n",state);
				return buf;
			}
	}
}

static int my_init(void)
{
	struct task_struct *task;
	unsigned int process_count=0;
	pr_info("%s:\n",__func__);
	
	for(task=current;task!=&init_task;task=task->parent){
		pr_info("process: %s\t  PID:[%d]\t state:%s\n",task->comm,task->pid,get_task_state(task->state));
		process_count++;
	}
	pr_info("number of processes:%u\n",process_count);
	return 0;
}

static void my_exit(void)
{
	pr_info("%s:in exit\n",__func__);
}

MODULE_LICENSE("GPL");
module_init(my_init);
module_exit(my_exit);
