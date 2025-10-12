#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/timekeeping.h>
#include<linux/rtc.h>
#include<linux/time.h>

static int __init hello_init(void)
{
        time64_t now;
        struct tm tm_now;

        now = ktime_get_real_seconds();

        time64_to_tm(now,0,&tm_now);

        pr_info("PREETHI\n");

        pr_info("CURRENT TIME: %04ld-%02d-%02d %02d:%02d:%02d\n",
                        tm_now.tm_year + 1900,
                        tm_now.tm_mon + 1,
                        tm_now.tm_mday,
                        tm_now.tm_hour,
                        tm_now.tm_min,
                        tm_now.tm_sec);


        return 0;
}

static void __exit hello_exit(void)
{
        pr_info("GOODBYE\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PREETHI");
