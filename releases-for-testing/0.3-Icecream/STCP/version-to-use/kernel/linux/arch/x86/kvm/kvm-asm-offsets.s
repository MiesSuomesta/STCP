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
	.file	"kvm-asm-offsets.c"
	.p2align	4, 0x90                         # -- Begin function common
	.type	common,@function
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
common:                                 # @common
.Lfunc_begin0:
# %bb.0:
	endbr64
	callq	__fentry__
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->SVM_vcpu_arch_regs $296 offsetof(struct vcpu_svm, vcpu.arch.regs)"
	#NO_APP
	#APP

	.ascii	"->SVM_current_vmcb $6488 offsetof(struct vcpu_svm, current_vmcb)"
	#NO_APP
	#APP

	.ascii	"->SVM_spec_ctrl $6536 offsetof(struct vcpu_svm, spec_ctrl)"
	#NO_APP
	#APP

	.ascii	"->SVM_vmcb01 $6456 offsetof(struct vcpu_svm, vmcb01)"
	#NO_APP
	#APP

	.ascii	"->KVM_VMCB_pa $8 offsetof(struct kvm_vmcb_info, pa)"
	#NO_APP
	#APP

	.ascii	"->SD_save_area_pa $32 offsetof(struct svm_cpu_data, save_area_pa)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->VMX_vcpu_arch_regs $296 offsetof(struct vcpu_vmx, vcpu.arch.regs)"
	#NO_APP
	#APP

	.ascii	"->VMX_spec_ctrl $6792 offsetof(struct vcpu_vmx, spec_ctrl)"
	#NO_APP
	cs
	jmp	__x86_return_thunk              # TAILCALL
.Lfunc_end0:
	.size	common, .Lfunc_end0-common
	.section	__patchable_function_entries,"awo",@progbits,common
	.p2align	3, 0x0
	.quad	.Ltmp0
                                        # -- End function
	.ident	"Debian clang version 19.1.7 (3+b1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym common
