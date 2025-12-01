#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>   // Required for strlen()

// This function is implemented in mblog driver and exported using EXPORT_SYMBOL
extern void mblog_write_kernel(const char *msg, size_t len);

// Module initialization function (runs when module is inserted)
static int __init klogger_init(void)
{
    char buf[] = "hello from kernel module\n";

    // Write log message directly into mblog kernel buffer
    mblog_write_kernel(buf, strlen(buf));

    pr_info("klogger inserted\n");
    return 0;
}

// Module exit function (runs when module is removed)
static void __exit klogger_exit(void)
{
    char buf1[] = "exit message from module\n";

    // Write another log message before module unloads
    mblog_write_kernel(buf1, strlen(buf1));

    pr_info("klogger removed\n");
}

// Register init and exit functions
module_init(klogger_init);
module_exit(klogger_exit);

MODULE_LICENSE("GPL");
