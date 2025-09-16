#include <linux/init.h>
 #include <linux/module.h>
 
 MODULE_LICENSE("GPL");
 MODULE_AUTHOR("You");
 MODULE_DESCRIPTION("Core module exporting log function");
 
 void core_log_message(const char *msg)
 {
 printk(KERN_INFO "[core_driver] Log: %s\n", msg);
 }
 EXPORT_SYMBOL(core_log_message); // This makes it visible to other modules
 
 static int __init core_init(void)
 {
 printk(KERN_INFO "core_driver loaded\n");
 return 0;
 }
 
 static void __exit core_exit(void)
 {
 printk(KERN_INFO "core_driver unloaded\n");
 }
 
 module_init(core_init);
 module_exit(core_exit);
