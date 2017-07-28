#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x9d35aeec, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xb18d97c, __VMLINUX_SYMBOL_STR(param_ops_long) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xe16b0d30, __VMLINUX_SYMBOL_STR(unregister_jprobe) },
	{ 0x177758bc, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xb1bedeb7, __VMLINUX_SYMBOL_STR(register_jprobe) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x211f68f1, __VMLINUX_SYMBOL_STR(getnstimeofday64) },
	{ 0x1b9aca3f, __VMLINUX_SYMBOL_STR(jprobe_return) },
	{ 0xb742fd7, __VMLINUX_SYMBOL_STR(simple_strtol) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xe914e41e, __VMLINUX_SYMBOL_STR(strcpy) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x3356b90b, __VMLINUX_SYMBOL_STR(cpu_tss) },
	{ 0x9166fada, __VMLINUX_SYMBOL_STR(strncpy) },
	{ 0x2142697b, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x8a9809d6, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "FCBA137BA59FDC51BFAD5B4");
