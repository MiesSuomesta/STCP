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

KSYMTAB_FUNC(stcp_proto_register, "_gpl", "");
KSYMTAB_FUNC(stcp_proto_unregister, "_gpl", "");
KSYMTAB_FUNC(stcp_kgetrandom, "_gpl", "");

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "24279A65D6986C8B0EB4208");
