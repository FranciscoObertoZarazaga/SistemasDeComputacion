// my_module.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/utsname.h>

static int __init my_module_init(void) {
    printk(KERN_INFO "Loaded module on: %s\n", utsname()->nodename);
    return 0;
}

static void __exit my_module_exit(void) {
    printk(KERN_INFO "Unloaded module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fran");
MODULE_DESCRIPTION("My first module");
