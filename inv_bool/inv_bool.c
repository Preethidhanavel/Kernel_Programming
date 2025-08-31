#include<linux/init.h>        // For module_init and module_exit macros
#include<linux/module.h>      // For module related macros
#include<linux/moduleparam.h> // For passing parameters to module

// Define a boolean parameter 
static bool param_invbool = false;

// Register the parameter as "invbool"
module_param(param_invbool, invbool, S_IRUGO);

// Init function 
static int my_init(void)
{
    // Print parameter value 
    pr_info("param_invbool: %s\n", param_invbool ? "true" : "false");
    return 0; // success
}

// Exit function 
static void my_exit(void)
{
    return; // nothing to cleanup
}

// Register init and exit functions
module_init(my_init);
module_exit(my_exit);

// Author and license info
MODULE_AUTHOR("linux trainer");
MODULE_LICENSE("GPL");
