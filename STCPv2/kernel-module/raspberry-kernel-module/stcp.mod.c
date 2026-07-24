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
	{ 0xf5d40bf9, "_copy_to_iter" },
	{ 0x30f9bed4, "param_ops_uint" },
	{ 0x3b03a4fd, "proc_create" },
	{ 0x608741b5, "__init_swait_queue_head" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x51a511eb, "_raw_write_lock_bh" },
	{ 0x5109de39, "kernel_bind" },
	{ 0x62737e1d, "sock_unregister" },
	{ 0x037a0cba, "kfree" },
	{ 0x054496b4, "schedule_timeout_interruptible" },
	{ 0xe7481d45, "seq_lseek" },
	{ 0xe7ab1ecc, "_raw_write_unlock_bh" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0xe2964344, "__wake_up" },
	{ 0x264b2149, "kernel_accept" },
	{ 0x97ee77e6, "kernel_recvmsg" },
	{ 0xf7431702, "wake_up_process" },
	{ 0x92997ed8, "_printk" },
	{ 0x76bc4923, "proto_unregister" },
	{ 0xb29f6a64, "___ratelimit" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0x01000e51, "schedule" },
	{ 0x147c3f2e, "chacha20poly1305_encrypt" },
	{ 0xa901fe66, "sock_register" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x0296695f, "refcount_warn_saturate" },
	{ 0x5c181841, "proto_register" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x59932123, "system_dfl_wq" },
	{ 0xbf1453cd, "init_net" },
	{ 0x9d8f35e8, "sk_free" },
	{ 0x97486609, "kernel_sock_shutdown" },
	{ 0x4dfa8d4b, "mutex_lock" },
	{ 0xd0760fc0, "kfree_sensitive" },
	{ 0xf74bb274, "mod_delayed_work_on" },
	{ 0x7412ed5b, "kvfree_sensitive" },
	{ 0xa19a38cd, "kthread_stop" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xcbe68c22, "sk_alloc" },
	{ 0x67cec549, "proc_mkdir" },
	{ 0x4f4b6585, "kernel_connect" },
	{ 0xc792c9c5, "sock_no_socketpair" },
	{ 0x25974000, "wait_for_completion" },
	{ 0x9d0129f8, "kmemdup_noprof" },
	{ 0xe7b262b1, "sock_no_mmap" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x93d6dd8c, "complete_all" },
	{ 0x429409ef, "proc_remove" },
	{ 0x0fbc93a6, "kthread_create_on_node" },
	{ 0xce248c5b, "sk_common_release" },
	{ 0x7fcbbcf0, "seq_read" },
	{ 0xd0014f37, "curve25519" },
	{ 0x03748cd2, "kernel_listen" },
	{ 0xc20134e7, "chacha20poly1305_decrypt" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x9fa7184a, "cancel_delayed_work_sync" },
	{ 0xce63f2a8, "sock_create_kern" },
	{ 0x448b58f4, "param_ops_bool" },
	{ 0x4ec25975, "seq_write" },
	{ 0x24bcf7cb, "__kmalloc_cache_noprof" },
	{ 0x5191bfd2, "sock_no_getname" },
	{ 0xaf27752a, "seq_printf" },
	{ 0xffeedf6a, "delayed_work_timer_fn" },
	{ 0x09f238cd, "_copy_from_iter" },
	{ 0x90460487, "sock_release" },
	{ 0xce70632b, "sock_gettstamp" },
	{ 0x791e3f85, "sock_no_ioctl" },
	{ 0x9906adb2, "single_release" },
	{ 0xf9ddb5d9, "timer_init_key" },
	{ 0x83d03362, "curve25519_generate_public" },
	{ 0xa65c6def, "alt_cb_patch_nops" },
	{ 0x41ed3709, "get_random_bytes" },
	{ 0x2fe5c6f8, "tcp_sock_set_nodelay" },
	{ 0x742578a5, "wait_for_random_bytes" },
	{ 0x9d0c7d6d, "iov_iter_revert" },
	{ 0x7aa1756e, "kvfree" },
	{ 0xa405108c, "single_open" },
	{ 0x6a601457, "kernel_sendmsg" },
	{ 0x792b67cd, "__kvmalloc_node_noprof" },
	{ 0xf9a482f9, "msleep" },
	{ 0xed60946e, "sock_init_data" },
	{ 0x94db548b, "kmalloc_caches" },
	{ 0xf23aabc1, "module_layout" },
};

MODULE_INFO(depends, "libchacha20poly1305,libcurve25519");


MODULE_INFO(srcversion, "D0CD67BF5D7A9BC07A14A7E");
