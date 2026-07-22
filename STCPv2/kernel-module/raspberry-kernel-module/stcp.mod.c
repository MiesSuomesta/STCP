#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

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



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf80ba5c8, "_copy_to_iter" },
	{ 0x265d6df7, "param_ops_uint" },
	{ 0x049a963b, "proc_create" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x51a511eb, "_raw_write_lock_bh" },
	{ 0x64473fc5, "kernel_bind" },
	{ 0x62737e1d, "sock_unregister" },
	{ 0x037a0cba, "kfree" },
	{ 0x054496b4, "schedule_timeout_interruptible" },
	{ 0x28f68296, "seq_lseek" },
	{ 0xe7ab1ecc, "_raw_write_unlock_bh" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xe2964344, "__wake_up" },
	{ 0x2db6b47e, "kernel_accept" },
	{ 0xd00410b7, "kernel_recvmsg" },
	{ 0x70b25da6, "wake_up_process" },
	{ 0x92997ed8, "_printk" },
	{ 0x780f2f23, "proto_unregister" },
	{ 0xb29f6a64, "___ratelimit" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x01000e51, "schedule" },
	{ 0x147c3f2e, "chacha20poly1305_encrypt" },
	{ 0x2cb6e9da, "sock_register" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x0296695f, "refcount_warn_saturate" },
	{ 0xc99b6ea8, "proto_register" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x59932123, "system_dfl_wq" },
	{ 0x1b990d99, "init_net" },
	{ 0x8722b738, "sk_free" },
	{ 0x6a1b98ee, "kernel_sock_shutdown" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0xd0760fc0, "kfree_sensitive" },
	{ 0xf74bb274, "mod_delayed_work_on" },
	{ 0x7412ed5b, "kvfree_sensitive" },
	{ 0x56fc4379, "kthread_stop" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x5d5538ed, "sk_alloc" },
	{ 0x511f0193, "proc_mkdir" },
	{ 0x36aded77, "kernel_connect" },
	{ 0xb79e5614, "sock_no_socketpair" },
	{ 0x25974000, "wait_for_completion" },
	{ 0x9d0129f8, "kmemdup_noprof" },
	{ 0x774ba213, "sock_no_mmap" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x93d6dd8c, "complete_all" },
	{ 0x52dd3722, "proc_remove" },
	{ 0xce5708e3, "kthread_create_on_node" },
	{ 0x806f99d4, "sk_common_release" },
	{ 0xb87ea521, "seq_read" },
	{ 0xd0014f37, "curve25519" },
	{ 0xc78b588d, "kernel_listen" },
	{ 0xc20134e7, "chacha20poly1305_decrypt" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0x3e0f7ca8, "sock_create_kern" },
	{ 0x522f8bd7, "param_ops_bool" },
	{ 0x190dd4c1, "seq_write" },
	{ 0x8320a874, "__kmalloc_cache_noprof" },
	{ 0xec8bfab0, "sock_no_getname" },
	{ 0xab1af175, "seq_printf" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0x628240a5, "_copy_from_iter" },
	{ 0x38a376cb, "sock_release" },
	{ 0xe54c0298, "sock_gettstamp" },
	{ 0xa27dab37, "sock_no_ioctl" },
	{ 0x09eae07a, "single_release" },
	{ 0xf9ddb5d9, "timer_init_key" },
	{ 0x83d03362, "curve25519_generate_public" },
	{ 0xa65c6def, "alt_cb_patch_nops" },
	{ 0x41ed3709, "get_random_bytes" },
	{ 0x5c132489, "tcp_sock_set_nodelay" },
	{ 0x742578a5, "wait_for_random_bytes" },
	{ 0x86cdcc7d, "iov_iter_revert" },
	{ 0x7aa1756e, "kvfree" },
	{ 0x10421460, "single_open" },
	{ 0xae8e7165, "kernel_sendmsg" },
	{ 0x792b67cd, "__kvmalloc_node_noprof" },
	{ 0xf9a482f9, "msleep" },
	{ 0x80aa4cf9, "sock_init_data" },
	{ 0x5bd716e6, "kmalloc_caches" },
	{ 0x91d66ee9, "module_layout" },
};

MODULE_INFO(depends, "libchacha20poly1305,libcurve25519");


MODULE_INFO(srcversion, "F4D5ACCCA8CDAEE7617A477");
