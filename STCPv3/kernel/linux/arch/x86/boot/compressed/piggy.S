.section ".rodata..compressed","a",@progbits
.globl z_input_len
z_input_len = 25635152
.globl z_output_len
z_output_len = 100995468
.globl input_data, input_data_end
input_data:
.incbin "arch/x86/boot/compressed/vmlinux.bin.zst"
input_data_end:
.section ".rodata","a",@progbits
.globl input_len
input_len:
	.long 25635152
.globl output_len
output_len:
	.long 100995468
