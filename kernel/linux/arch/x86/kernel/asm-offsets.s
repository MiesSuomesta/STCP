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
	.file	"asm-offsets.c"
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

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->KVM_STEAL_TIME_preempted $16 offsetof(struct kvm_steal_time, preempted)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->pt_regs_bx $40 offsetof(struct pt_regs, bx)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_cx $88 offsetof(struct pt_regs, cx)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_dx $96 offsetof(struct pt_regs, dx)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_sp $152 offsetof(struct pt_regs, sp)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_bp $32 offsetof(struct pt_regs, bp)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_si $104 offsetof(struct pt_regs, si)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_di $112 offsetof(struct pt_regs, di)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r8 $72 offsetof(struct pt_regs, r8)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r9 $64 offsetof(struct pt_regs, r9)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r10 $56 offsetof(struct pt_regs, r10)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r11 $48 offsetof(struct pt_regs, r11)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r12 $24 offsetof(struct pt_regs, r12)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r13 $16 offsetof(struct pt_regs, r13)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r14 $8 offsetof(struct pt_regs, r14)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_r15 $0 offsetof(struct pt_regs, r15)"
	#NO_APP
	#APP

	.ascii	"->pt_regs_flags $144 offsetof(struct pt_regs, flags)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->saved_context_cr0 $200 offsetof(struct saved_context, cr0)"
	#NO_APP
	#APP

	.ascii	"->saved_context_cr2 $208 offsetof(struct saved_context, cr2)"
	#NO_APP
	#APP

	.ascii	"->saved_context_cr3 $216 offsetof(struct saved_context, cr3)"
	#NO_APP
	#APP

	.ascii	"->saved_context_cr4 $224 offsetof(struct saved_context, cr4)"
	#NO_APP
	#APP

	.ascii	"->saved_context_gdt_desc $266 offsetof(struct saved_context, gdt_desc)"
	#NO_APP
	#APP

	.ascii	"->"
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
	.text
	.p2align	4, 0x90                         # -- Begin function common
	.type	common,@function
.Ltmp1:
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
.Lfunc_begin1:
# %bb.0:
	endbr64
	callq	__fentry__
	#APP

	.ascii	"->CPUINFO_x86 $1 offsetof(struct cpuinfo_x86, x86)"
	#NO_APP
	#APP

	.ascii	"->CPUINFO_x86_vendor $2 offsetof(struct cpuinfo_x86, x86_vendor)"
	#NO_APP
	#APP

	.ascii	"->CPUINFO_x86_model $0 offsetof(struct cpuinfo_x86, x86_model)"
	#NO_APP
	#APP

	.ascii	"->CPUINFO_x86_stepping $4 offsetof(struct cpuinfo_x86, x86_stepping)"
	#NO_APP
	#APP

	.ascii	"->CPUINFO_cpuid_level $40 offsetof(struct cpuinfo_x86, cpuid_level)"
	#NO_APP
	#APP

	.ascii	"->CPUINFO_x86_capability $48 offsetof(struct cpuinfo_x86, x86_capability)"
	#NO_APP
	#APP

	.ascii	"->CPUINFO_x86_vendor_id $144 offsetof(struct cpuinfo_x86, x86_vendor_id)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->TASK_threadsp $5624 offsetof(struct task_struct, thread.sp)"
	#NO_APP
	#APP

	.ascii	"->TASK_stack_canary $2560 offsetof(struct task_struct, stack_canary)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->pbe_address $0 offsetof(struct pbe, address)"
	#NO_APP
	#APP

	.ascii	"->pbe_orig_address $8 offsetof(struct pbe, orig_address)"
	#NO_APP
	#APP

	.ascii	"->pbe_next $16 offsetof(struct pbe, next)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_ax $44 offsetof(struct sigcontext_32, ax)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_bx $32 offsetof(struct sigcontext_32, bx)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_cx $40 offsetof(struct sigcontext_32, cx)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_dx $36 offsetof(struct sigcontext_32, dx)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_si $20 offsetof(struct sigcontext_32, si)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_di $16 offsetof(struct sigcontext_32, di)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_bp $24 offsetof(struct sigcontext_32, bp)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_sp $28 offsetof(struct sigcontext_32, sp)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_ip $56 offsetof(struct sigcontext_32, ip)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_es $8 offsetof(struct sigcontext_32, es)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_cs $60 offsetof(struct sigcontext_32, cs)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_ss $72 offsetof(struct sigcontext_32, ss)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_ds $12 offsetof(struct sigcontext_32, ds)"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGCONTEXT_flags $64 offsetof(struct sigcontext_32, flags)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->IA32_SIGFRAME_sigcontext $8 offsetof(struct sigframe_ia32, sc)"
	#NO_APP
	#APP

	.ascii	"->IA32_RT_SIGFRAME_sigcontext $164 offsetof(struct rt_sigframe_ia32, uc.uc_mcontext)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->XEN_vcpu_info_mask $1 offsetof(struct vcpu_info, evtchn_upcall_mask)"
	#NO_APP
	#APP

	.ascii	"->XEN_vcpu_info_pending $0 offsetof(struct vcpu_info, evtchn_upcall_pending)"
	#NO_APP
	#APP

	.ascii	"->XEN_vcpu_info_arch_cr2 $16 offsetof(struct vcpu_info, arch.cr2)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_rcx $0 offsetof(struct tdx_module_args, rcx)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_rdx $8 offsetof(struct tdx_module_args, rdx)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r8 $16 offsetof(struct tdx_module_args, r8)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r9 $24 offsetof(struct tdx_module_args, r9)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r10 $32 offsetof(struct tdx_module_args, r10)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r11 $40 offsetof(struct tdx_module_args, r11)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r12 $48 offsetof(struct tdx_module_args, r12)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r13 $56 offsetof(struct tdx_module_args, r13)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r14 $64 offsetof(struct tdx_module_args, r14)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_r15 $72 offsetof(struct tdx_module_args, r15)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_rbx $80 offsetof(struct tdx_module_args, rbx)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_rdi $88 offsetof(struct tdx_module_args, rdi)"
	#NO_APP
	#APP

	.ascii	"->TDX_MODULE_rsi $96 offsetof(struct tdx_module_args, rsi)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->BP_scratch $484 offsetof(struct boot_params, scratch)"
	#NO_APP
	#APP

	.ascii	"->BP_secure_boot $492 offsetof(struct boot_params, secure_boot)"
	#NO_APP
	#APP

	.ascii	"->BP_loadflags $529 offsetof(struct boot_params, hdr.loadflags)"
	#NO_APP
	#APP

	.ascii	"->BP_hardware_subarch $572 offsetof(struct boot_params, hdr.hardware_subarch)"
	#NO_APP
	#APP

	.ascii	"->BP_version $518 offsetof(struct boot_params, hdr.version)"
	#NO_APP
	#APP

	.ascii	"->BP_kernel_alignment $560 offsetof(struct boot_params, hdr.kernel_alignment)"
	#NO_APP
	#APP

	.ascii	"->BP_init_size $608 offsetof(struct boot_params, hdr.init_size)"
	#NO_APP
	#APP

	.ascii	"->BP_pref_address $600 offsetof(struct boot_params, hdr.pref_address)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->PTREGS_SIZE $168 sizeof(struct pt_regs)"
	#NO_APP
	#APP

	.ascii	"->C_PTREGS_SIZE $120 offsetof(struct pt_regs, orig_ax)"
	#NO_APP
	#APP

	.ascii	"->TLB_STATE_user_pcid_flush_mask $22 offsetof(struct tlb_state, user_pcid_flush_mask)"
	#NO_APP
	#APP

	.ascii	"->CPU_ENTRY_AREA_entry_stack $4096 offsetof(struct cpu_entry_area, entry_stack_page)"
	#NO_APP
	#APP

	.ascii	"->SIZEOF_entry_stack $4096 sizeof(struct entry_stack)"
	#NO_APP
	#APP

	.ascii	"->MASK_entry_stack $-4096 (~(sizeof(struct entry_stack) - 1))"
	#NO_APP
	#APP

	.ascii	"->TSS_sp0 $4 offsetof(struct tss_struct, x86_tss.sp0)"
	#NO_APP
	#APP

	.ascii	"->TSS_sp1 $12 offsetof(struct tss_struct, x86_tss.sp1)"
	#NO_APP
	#APP

	.ascii	"->TSS_sp2 $20 offsetof(struct tss_struct, x86_tss.sp2)"
	#NO_APP
	#APP

	.ascii	"->"
	#NO_APP
	#APP

	.ascii	"->ALT_INSTR_SIZE $14 sizeof(struct alt_instr)"
	#NO_APP
	#APP

	.ascii	"->EXTABLE_SIZE $12 sizeof(struct exception_table_entry)"
	#NO_APP
	cs
	jmp	__x86_return_thunk              # TAILCALL
.Lfunc_end1:
	.size	common, .Lfunc_end1-common
	.section	__patchable_function_entries,"awo",@progbits,common
	.p2align	3, 0x0
	.quad	.Ltmp1
                                        # -- End function
	.ident	"Debian clang version 19.1.7 (3+b1)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym common
