	.text
	.section	.note.gnu.property,"a",@note
	.p2align	3, 0x0
	.long	4
	.long	16
	.long	5
	.asciz	"GNU"
	.long	3221225474
	.long	4
	.long	1
	.p2align	3, 0x0
	.text
	.file	"bounds.c"
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
.Ltmp0:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
main:                                   # @main
.Lfunc_begin0:
# %bb.0:
	endbr64
	callq	__fentry__
	pushq	%rbp
	movq	%rsp, %rbp
	andq	$-8, %rsp
	#APP

	.ascii	"->NR_PAGEFLAGS $25 __NR_PAGEFLAGS"
	#NO_APP
	#APP

	.ascii	"->MAX_NR_ZONES $5 __MAX_NR_ZONES"
	#NO_APP
	#APP

	.ascii	"->NR_CPUS_BITS $13 order_base_2(CONFIG_NR_CPUS)"
	#NO_APP
	#APP

	.ascii	"->SPINLOCK_SIZE $4 sizeof(spinlock_t)"
	#NO_APP
	#APP

	.ascii	"->LRU_GEN_WIDTH $3 order_base_2(MAX_NR_GENS + 1)"
	#NO_APP
	#APP

	.ascii	"->__LRU_REFS_WIDTH $2 MAX_NR_TIERS - 2"
	#NO_APP
	xorl	%eax, %eax
	movq	%rbp, %rsp
	popq	%rbp
	cs
	jmp	__x86_return_thunk              # TAILCALL
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.section	__patchable_function_entries,"awo",@progbits,main
	.p2align	3, 0x0
	.quad	.Ltmp0
                                        # -- End function
	.ident	"Debian clang version 19.1.7 (3+b1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
