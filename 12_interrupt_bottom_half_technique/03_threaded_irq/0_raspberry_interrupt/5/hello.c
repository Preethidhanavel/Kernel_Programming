#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/kthread.h>


struct task_struct *blink_thread;
static unsigned int gpio_led = 14;

MODULE_LICENSE("GPL");

static int led_thread(void *data)
{
        while(!kthread_should_stop()) {
		gpio_set_value(gpio_led, 1);
		ssleep(1);
		gpio_set_value(gpio_led, 0);
		ssleep(1);
        }
        return 0;
}


static int test_hello_init(void)
{
	pr_info("%s: In init\n", __func__);

	if (!gpio_is_valid(gpio_led)){
		pr_info("Invalid LED GPIO:%d\n", gpio_led);
		return -ENODEV;
	}

	pr_info("gpio led:%d is valid\n", gpio_led);
	if (gpio_request(gpio_led, "my_led")) {
		pr_info("GPIO Request failed on gpio:%d\n", gpio_led);
		return -EINVAL;
	}
	pr_info("GPIO Request successful on gpio:%d\n", gpio_led);

	gpio_direction_output(gpio_led, 0);
	blink_thread = kthread_run(led_thread, NULL, "ledthread");
	
	return 0;
}

static void test_hello_exit(void)
{
	pr_info("%s: In exit\n", __func__);
	kthread_stop(blink_thread);
	gpio_free(gpio_led);
}

module_init(test_hello_init);
module_exit(test_hello_exit);
