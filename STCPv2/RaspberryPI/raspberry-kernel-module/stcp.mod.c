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
	{ 0x71ec8ad6, "param_ops_uint" },
	{ 0x049a963b, "proc_create" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x51a511eb, "_raw_write_lock_bh" },
	{ 0xb713b22b, "kernel_bind" },
	{ 0x62737e1d, "sock_unregister" },
	{ 0x037a0cba, "kfree" },
	{ 0x054496b4, "schedule_timeout_interruptible" },
	{ 0x28f68296, "seq_lseek" },
	{ 0xe7ab1ecc, "_raw_write_unlock_bh" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xe2964344, "__wake_up" },
	{ 0x7cf708d0, "kernel_accept" },
	{ 0xef2d7286, "kernel_recvmsg" },
	{ 0x8582daf7, "wake_up_process" },
	{ 0x92997ed8, "_printk" },
	{ 0x4b4da8b3, "proto_unregister" },
	{ 0xb29f6a64, "___ratelimit" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x01000e51, "schedule" },
	{ 0x147c3f2e, "chacha20poly1305_encrypt" },
	{ 0xe784bc37, "sock_register" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x0296695f, "refcount_warn_saturate" },
	{ 0x857880c9, "proto_register" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x59932123, "system_dfl_wq" },
	{ 0xcea12631, "init_net" },
	{ 0x174ffc87, "kernel_getpeername" },
	{ 0x2acb4e4a, "kernel_getsockname" },
	{ 0xed6a4cc6, "sk_free" },
	{ 0x588f634c, "kernel_sock_shutdown" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0xd0760fc0, "kfree_sensitive" },
	{ 0xf74bb274, "mod_delayed_work_on" },
	{ 0x7412ed5b, "kvfree_sensitive" },
	{ 0x56fc4379, "kthread_stop" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0x52e309e6, "sk_alloc" },
	{ 0x511f0193, "proc_mkdir" },
	{ 0xe6debb74, "kernel_connect" },
	{ 0x13374535, "sock_no_socketpair" },
	{ 0x25974000, "wait_for_completion" },
	{ 0x9d0129f8, "kmemdup_noprof" },
	{ 0x6f7e3f3d, "sock_no_mmap" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x93d6dd8c, "complete_all" },
	{ 0x52dd3722, "proc_remove" },
	{ 0xce5708e3, "kthread_create_on_node" },
	{ 0x504aafec, "sk_common_release" },
	{ 0xb87ea521, "seq_read" },
	{ 0xd0014f37, "curve25519" },
	{ 0xcda0ec2f, "kernel_listen" },
	{ 0xc20134e7, "chacha20poly1305_decrypt" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0xbbcced84, "sock_create_kern" },
	{ 0x059e6cf6, "param_ops_bool" },
	{ 0x190dd4c1, "seq_write" },
	{ 0x8320a874, "__kmalloc_cache_noprof" },
	{ 0x04cb5dbf, "sock_no_getname" },
	{ 0xab1af175, "seq_printf" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0x628240a5, "_copy_from_iter" },
	{ 0xeaf20955, "sock_release" },
	{ 0x8529aabd, "sock_gettstamp" },
	{ 0x31bfa013, "sock_no_ioctl" },
	{ 0x09eae07a, "single_release" },
	{ 0xf9ddb5d9, "timer_init_key" },
	{ 0x83d03362, "curve25519_generate_public" },
	{ 0xa65c6def, "alt_cb_patch_nops" },
	{ 0x41ed3709, "get_random_bytes" },
	{ 0xbab30a89, "tcp_sock_set_nodelay" },
	{ 0x742578a5, "wait_for_random_bytes" },
	{ 0x86cdcc7d, "iov_iter_revert" },
	{ 0x7aa1756e, "kvfree" },
	{ 0x10421460, "single_open" },
	{ 0xafa12549, "kernel_sendmsg" },
	{ 0x792b67cd, "__kvmalloc_node_noprof" },
	{ 0xf9a482f9, "msleep" },
	{ 0x24ec43a0, "sock_init_data" },
	{ 0x98fcaf34, "kmalloc_caches" },
	{ 0x91d66ee9, "module_layout" },
};

MODULE_INFO(depends, "libchacha20poly1305,libcurve25519");


MODULE_INFO(srcversion, "FF5EDC8136AD1DED1B0C865");
