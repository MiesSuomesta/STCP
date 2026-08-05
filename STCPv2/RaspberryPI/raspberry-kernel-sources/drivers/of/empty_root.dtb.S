#include <asm-generic/vmlinux.lds.h>
.section .rodata,"a"
.balign STRUCT_ALIGNMENT
.global __dtb_empty_root_begin
__dtb_empty_root_begin:
.incbin "drivers/of/empty_root.dtb" 
.global __dtb_empty_root_end
__dtb_empty_root_end:
.balign STRUCT_ALIGNMENT
