#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

struct mutex *mylock;

static int __init test_hello_init(void)
{
    mylock = kmalloc(sizeof(mylock), GFP_KERNEL);
    mutex_init(mylock);
    mutex_lock(mylock);
    pr_info("Starting critical region\n");
    pr_info("Ending critical region\n");
    mutex_unlock(mylock);
    kfree(mylock);
    return -1;
}

static void __exit test_hello_exit(void)
{
}

module_init(test_hello_init);
module_exit(test_hello_exit);
