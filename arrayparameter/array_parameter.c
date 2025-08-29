#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/moduleparam.h>

MODULE_LICENSE("GPL");
int param_array[4];
static int count =0;

module_param_array(param_array,int,&count,S_IWUSR|S_IRUSR);


static int test_arguments_init(void)
{
	int i;
	printk(KERN_INFO"%s: In init\n",__func__);
	printk(KERN_INFO"%s: count:%d\n",__func__,count);
	for(i=0;i<(sizeof(param_array)/sizeof(param_array[i]));i++)
		printk(KERN_INFO"%d:\t %d\n",i,param_array[i]);
	return 0;
}

static void test_arguments_exit(void)
{
	printk(KERN_INFO"%s: In exit\n",__func__);
}

module_init(test_arguments_init);
module_exit(test_arguments_exit);
