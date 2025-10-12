#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>


static int arr[3] = {0, 0, 0};   
static int count = 0;               


module_param_array(arr, int, &count, 0660);
MODULE_PARM_DESC(arr, "An array of 3 integers");

static int __init array_module_init(void)
{
    int i;
    int sum = 0;
    int avg;

    printk(KERN_INFO "array_module loaded.\n");
    printk(KERN_INFO "Number of elements passed: %d\n", count);

    for(i = 0; i < count; i++)
    {
        printk(KERN_INFO "arr[%d] = %d\n", i, arr[i]);
        sum += arr[i];
    }

    if(count > 0)
        avg = sum / count;
    else
        avg = 0;

    printk(KERN_INFO "Average = %d\n", avg);
    return 0;
}

static void __exit array_module_exit(void)
{
    printk(KERN_INFO "array_module removed.\n");
}

module_init(array_module_init);
module_exit(array_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
MODULE_DESCRIPTION("Kernel module that calculates average of an integer array");


