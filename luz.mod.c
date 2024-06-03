#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x122c3a7e, "_printk" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x6f33fed0, "class_create" },
	{ 0x7188b409, "device_create" },
	{ 0xe6f9d1e8, "cdev_init" },
	{ 0xe8f65a0d, "cdev_add" },
	{ 0xfbade11, "i2c_get_adapter" },
	{ 0x8a01c137, "i2c_new_client_device" },
	{ 0x218db5b1, "i2c_put_adapter" },
	{ 0x93e67167, "device_destroy" },
	{ 0xcbedc829, "class_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x2a7e9b89, "try_module_get" },
	{ 0x4a3deea5, "sock_create" },
	{ 0xac5fcec0, "in4_pton" },
	{ 0x59d18360, "kthread_create_on_node" },
	{ 0xb765553d, "wake_up_process" },
	{ 0xb353ce91, "sock_release" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x98cf60b3, "strlen" },
	{ 0xdcb764ad, "memset" },
	{ 0xd37b4ff3, "kernel_sendmsg" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xf9a482f9, "msleep" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xde5abbd6, "i2c_transfer_buffer_flags" },
	{ 0x20137b52, "module_put" },
	{ 0x59c08c5b, "kthread_stop" },
	{ 0xb07237c5, "i2c_unregister_device" },
	{ 0xffc6fd5, "cdev_del" },
	{ 0xe478ef45, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B261896AA16991F9B172805");
