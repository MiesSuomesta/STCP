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
	.file	"rq-offsets.c"
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

	.ascii	"->RQ_nr_pinned $3504 offsetof(struct rq, nr_pinned)"
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
