#include<linux/init.h>
#include<linux/module.h>
#include<linux/moduleparam.h>

static bool param_invbool = false;
module_param(param_invbool, invbool, S_IRUGO);

static int my_init(void)
{
	pr_info("param_invbool: %s\n",param_invbool ? "true" : "false");
	return 0;
}

static void my_exit(void)
{
	return ;

}
module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("linux trainer");
MODULE_LICENSE("GPL");
