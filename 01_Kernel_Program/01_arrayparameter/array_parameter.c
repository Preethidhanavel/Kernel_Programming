#include<linux/kernel.h>      // For kernel functions like printk
#include<linux/module.h>      // For module macros 
#include<linux/moduleparam.h> // For passing arguments to module from insmod

MODULE_LICENSE("GPL");        // License declaration 

// Declare an integer array that can take up to parameters
int param_array[4];

// Variable to store how many arguments were actually passed
static int count = 0;

// Declare a module parameter as an array
module_param_array(param_array, int, &count, S_IWUSR | S_IRUSR);

// Module init function 
static int test_arguments_init(void)
{
    int i;
    printk(KERN_INFO "%s: In init\n", __func__);   // Print init message
    printk(KERN_INFO "%s: count:%d\n", __func__, count); // Print how many args passed

    // Loop through array and print elements
    for (i = 0; i < (sizeof(param_array)/sizeof(param_array[i])); i++)
        printk(KERN_INFO "%d:\t %d\n", i, param_array[i]);

    return 0; // Return success
}

// Module exit function 
static void test_arguments_exit(void)
{
    printk(KERN_INFO "%s: In exit\n", __func__);   // Print exit message
}

// Register init and exit functions
module_init(test_arguments_init);
module_exit(test_arguments_exit);
