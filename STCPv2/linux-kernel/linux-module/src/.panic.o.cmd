savedcmd_src/panic.o := clang -Wp,-MMD,src/.panic.o.d -nostdinc -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/uapi -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated/uapi -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/generated/uapi -include /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler-version.h -include /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kconfig.h -include /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler_types.h -D__KERNEL__ --target=x86_64-linux-gnu -fintegrated-as -Werror=unknown-warning-option -Werror=ignored-optimization-argument -Werror=option-ignored -Werror=unused-command-line-argument -fshort-wchar -funsigned-char -fno-common -fno-PIE -fno-strict-aliasing -std=gnu11 -fms-extensions -Wno-gnu -Wno-microsoft-anon-tag -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -mno-sse4a -fcf-protection=branch -fno-jump-tables -m64 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mstack-alignment=8 -mskip-rax-setup -march=x86-64 -mtune=generic -mno-red-zone -mcmodel=kernel -mstack-protector-guard-reg=gs -mstack-protector-guard-symbol=__ref_stack_chk_guard -Wno-sign-compare -fno-asynchronous-unwind-tables -mretpoline-external-thunk -mindirect-branch-cs-prefix -mfunction-return=thunk-extern -mharden-sls=all -fpatchable-function-entry=16,16 -fno-delete-null-pointer-checks -O2 -fstack-protector-strong -ftrivial-auto-var-init=zero -fno-stack-clash-protection -pg -mfentry -DCC_USING_NOP_MCOUNT -DCC_USING_FENTRY -falign-functions=16 -fstrict-flex-arrays=3 -fno-strict-overflow -fno-stack-check -fno-builtin-wcslen -Wall -Wextra -Wundef -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Werror=strict-prototypes -Wno-format-security -Wno-trigraphs -Wno-frame-address -Wno-address-of-packed-member -Wmissing-declarations -Wmissing-prototypes -Wframe-larger-than=2048 -Wno-format-overflow-non-kprintf -Wno-format-truncation-non-kprintf -Wno-type-limits -Wno-pointer-sign -Wcast-function-type -Wimplicit-fallthrough -Werror=date-time -Werror=incompatible-pointer-types -Wenum-conversion -Wunused -Wno-unused-but-set-variable -Wno-unused-const-variable -Wno-format-overflow -Wno-override-init -Wno-pointer-to-enum-cast -Wno-tautological-constant-out-of-range-compare -Wno-unaligned-access -Wno-enum-compare-conditional -Wno-missing-field-initializers -Wno-shift-negative-value -Wno-enum-enum-conversion -Wno-sign-compare -Wno-unused-parameter -g -I././include    -fsanitize=kernel-address -mllvm -asan-mapping-offset=0xdffffc0000000000  -mllvm -asan-instrumentation-with-call-threshold=10000  -mllvm -asan-stack=0    -mllvm -asan-globals=1  -mllvm -asan-kernel-mem-intrinsic-prefix=1  -DMODULE  -DKBUILD_BASENAME='"panic"' -DKBUILD_MODNAME='"stcp"' -D__KBUILD_MODNAME=stcp -c -o src/panic.o src/panic.c  

source_src/panic.o := src/panic.c

deps_src/panic.o := \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler-version.h \
    $(wildcard include/config/CC_VERSION_TEXT) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kconfig.h \
    $(wildcard include/config/CPU_BIG_ENDIAN) \
    $(wildcard include/config/BOOGER) \
    $(wildcard include/config/FOO) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler_types.h \
    $(wildcard include/config/DEBUG_INFO_BTF) \
    $(wildcard include/config/PAHOLE_HAS_BTF_TAG) \
    $(wildcard include/config/FUNCTION_ALIGNMENT) \
    $(wildcard include/config/CC_HAS_SANE_FUNCTION_ALIGNMENT) \
    $(wildcard include/config/X86_64) \
    $(wildcard include/config/ARM64) \
    $(wildcard include/config/LD_DEAD_CODE_DATA_ELIMINATION) \
    $(wildcard include/config/LTO_CLANG) \
    $(wildcard include/config/HAVE_ARCH_COMPILER_H) \
    $(wildcard include/config/KCSAN) \
    $(wildcard include/config/CC_HAS_ASSUME) \
    $(wildcard include/config/CC_HAS_COUNTED_BY) \
    $(wildcard include/config/FORTIFY_SOURCE) \
    $(wildcard include/config/UBSAN_BOUNDS) \
    $(wildcard include/config/CC_HAS_COUNTED_BY_PTR) \
    $(wildcard include/config/CC_HAS_MULTIDIMENSIONAL_NONSTRING) \
    $(wildcard include/config/CFI) \
    $(wildcard include/config/ARCH_USES_CFI_GENERIC_LLVM_PASS) \
    $(wildcard include/config/CC_HAS_BROKEN_COUNTED_BY_REF) \
    $(wildcard include/config/CC_HAS_ASM_INLINE) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler-context-analysis.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler_attributes.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler-clang.h \
    $(wildcard include/config/ARCH_USE_BUILTIN_BSWAP) \
    $(wildcard include/config/CLANG_VERSION) \
    $(wildcard include/config/CC_HAS_TYPEOF_UNQUAL) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/percpu_types.h \
    $(wildcard include/config/SMP) \
    $(wildcard include/config/CC_HAS_NAMED_AS) \
    $(wildcard include/config/USE_X86_SEG_SUPPORT) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/percpu_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/bug.h \
    $(wildcard include/config/GENERIC_BUG) \
    $(wildcard include/config/PRINTK) \
    $(wildcard include/config/BUG_ON_DATA_CORRUPTION) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/bug.h \
    $(wildcard include/config/DEBUG_BUGVERBOSE) \
    $(wildcard include/config/DEBUG_BUGVERBOSE_DETAILED) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/stringify.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/instrumentation.h \
    $(wildcard include/config/NOINSTR_VALIDATION) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/objtool.h \
    $(wildcard include/config/OBJTOOL) \
    $(wildcard include/config/FRAME_POINTER) \
    $(wildcard include/config/MITIGATION_UNRET_ENTRY) \
    $(wildcard include/config/MITIGATION_SRSO) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/objtool_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/types.h \
    $(wildcard include/config/HAVE_UID16) \
    $(wildcard include/config/UID16) \
    $(wildcard include/config/ARCH_DMA_ADDR_T_64BIT) \
    $(wildcard include/config/PHYS_ADDR_T_64BIT) \
    $(wildcard include/config/64BIT) \
    $(wildcard include/config/ARCH_32BIT_USTAT_F_TINODE) \
    $(wildcard include/config/KCOV) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated/uapi/asm/types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/int-ll64.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/int-ll64.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/uapi/asm/bitsperlong.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitsperlong.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/bitsperlong.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/posix_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/stddef.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/stddef.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/posix_types.h \
    $(wildcard include/config/X86_32) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/uapi/asm/posix_types_64.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/posix_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/annotate.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/asm.h \
    $(wildcard include/config/KPROBES) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/asm-offsets.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/generated/asm-offsets.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/extable_fixup_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/static_call_types.h \
    $(wildcard include/config/HAVE_STATIC_CALL) \
    $(wildcard include/config/HAVE_STATIC_CALL_INLINE) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler.h \
    $(wildcard include/config/TRACE_BRANCH_PROFILING) \
    $(wildcard include/config/PROFILE_ALL_BRANCHES) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated/asm/rwonce.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/rwonce.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kasan-checks.h \
    $(wildcard include/config/KASAN_GENERIC) \
    $(wildcard include/config/KASAN_SW_TAGS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kcsan-checks.h \
    $(wildcard include/config/KCSAN_WEAK_MEMORY) \
    $(wildcard include/config/KCSAN_IGNORE_ATOMICS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bug.h \
    $(wildcard include/config/BUG) \
    $(wildcard include/config/GENERIC_BUG_RELATIVE_POINTERS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/once_lite.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/panic.h \
    $(wildcard include/config/PANIC_TIMEOUT) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/stdarg.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/printk.h \
    $(wildcard include/config/MESSAGE_LOGLEVEL_DEFAULT) \
    $(wildcard include/config/CONSOLE_LOGLEVEL_DEFAULT) \
    $(wildcard include/config/CONSOLE_LOGLEVEL_QUIET) \
    $(wildcard include/config/EARLY_PRINTK) \
    $(wildcard include/config/PRINTK_INDEX) \
    $(wildcard include/config/DYNAMIC_DEBUG) \
    $(wildcard include/config/DYNAMIC_DEBUG_CORE) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/init.h \
    $(wildcard include/config/MEMORY_HOTPLUG) \
    $(wildcard include/config/HAVE_ARCH_PREL32_RELOCATIONS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/build_bug.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kern_levels.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/linkage.h \
    $(wildcard include/config/ARCH_USE_SYM_ANNOTATIONS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/export.h \
    $(wildcard include/config/MODVERSIONS) \
    $(wildcard include/config/GENDWARFKSYMS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/linkage.h \
    $(wildcard include/config/CALL_PADDING) \
    $(wildcard include/config/MITIGATION_RETHUNK) \
    $(wildcard include/config/MITIGATION_RETPOLINE) \
    $(wildcard include/config/MITIGATION_SLS) \
    $(wildcard include/config/FUNCTION_PADDING_BYTES) \
    $(wildcard include/config/UML) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/ibt.h \
    $(wildcard include/config/X86_KERNEL_IBT) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/ratelimit_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/bits.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/vdso/bits.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/vdso/const.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/const.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/bits.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/overflow.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/limits.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/limits.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/vdso/limits.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/const.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/param.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated/uapi/asm/param.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/param.h \
    $(wildcard include/config/HZ) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/param.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/spinlock_types_raw.h \
    $(wildcard include/config/DEBUG_SPINLOCK) \
    $(wildcard include/config/DEBUG_LOCK_ALLOC) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/spinlock_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/qspinlock_types.h \
    $(wildcard include/config/NR_CPUS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/qrwlock_types.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/uapi/asm/byteorder.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/byteorder/little_endian.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/byteorder/little_endian.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/swab.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/swab.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/uapi/asm/swab.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/byteorder/generic.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/lockdep_types.h \
    $(wildcard include/config/PROVE_RAW_LOCK_NESTING) \
    $(wildcard include/config/LOCKDEP) \
    $(wildcard include/config/LOCK_STAT) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/dynamic_debug.h \
    $(wildcard include/config/JUMP_LABEL) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/jump_label.h \
    $(wildcard include/config/HAVE_ARCH_JUMP_LABEL_RELATIVE) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/cleanup.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/err.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated/uapi/asm/errno.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/errno.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/asm-generic/errno-base.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/args.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/jump_label.h \
    $(wildcard include/config/HAVE_JUMP_LABEL_HACK) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/nops.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kernel.h \
    $(wildcard include/config/PREEMPT_VOLUNTARY_BUILD) \
    $(wildcard include/config/PREEMPT_DYNAMIC) \
    $(wildcard include/config/HAVE_PREEMPT_DYNAMIC_CALL) \
    $(wildcard include/config/HAVE_PREEMPT_DYNAMIC_KEY) \
    $(wildcard include/config/PREEMPT_) \
    $(wildcard include/config/DEBUG_ATOMIC_SLEEP) \
    $(wildcard include/config/MMU) \
    $(wildcard include/config/PROVE_LOCKING) \
    $(wildcard include/config/DYNAMIC_FTRACE) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/align.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/vdso/align.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/array_size.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/container_of.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/bitops.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/typecheck.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/kernel.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi/linux/sysinfo.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/generic-non-atomic.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/barrier.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/alternative.h \
    $(wildcard include/config/CALL_THUNKS) \
    $(wildcard include/config/MITIGATION_ITS) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/barrier.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/bitops.h \
    $(wildcard include/config/X86_CMOV) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/rmwcc.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/sched.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/arch_hweight.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/cpufeatures.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/const_hweight.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/instrumented-atomic.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/instrumented.h \
    $(wildcard include/config/DEBUG_ATOMIC) \
    $(wildcard include/config/DEBUG_ATOMIC_LARGEST_ALIGN) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kmsan-checks.h \
    $(wildcard include/config/KMSAN) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/instrumented-non-atomic.h \
    $(wildcard include/config/KCSAN_ASSUME_PLAIN_WRITES_ATOMIC) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/instrumented-lock.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/le.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/bitops/ext2-atomic-setbit.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kstrtox.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/log2.h \
    $(wildcard include/config/ARCH_HAS_ILOG2_U32) \
    $(wildcard include/config/ARCH_HAS_ILOG2_U64) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/math.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/asm/div64.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/asm-generic/div64.h \
    $(wildcard include/config/CC_OPTIMIZE_FOR_PERFORMANCE) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/minmax.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/sprintf.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/trace_printk.h \
    $(wildcard include/config/TRACING) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/instruction_pointer.h \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/util_macros.h \
    $(wildcard include/config/FOO_SUSPEND) \
  /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/wordpart.h \

src/panic.o: $(deps_src/panic.o)

$(deps_src/panic.o):
