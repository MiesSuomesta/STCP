savedcmd_src/stcp_rust_ffi.o := clang -Wp,-MMD,src/.stcp_rust_ffi.o.d -nostdinc -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/uapi -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/arch/x86/include/generated/uapi -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/uapi -I/usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/generated/uapi -include /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler-version.h -include /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/kconfig.h -include /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug-kasan-uus/include/linux/compiler_types.h -D__KERNEL__ --target=x86_64-linux-gnu -fintegrated-as -Werror=unknown-warning-option -Werror=ignored-optimization-argument -Werror=option-ignored -Werror=unused-command-line-argument -fshort-wchar -funsigned-char -fno-common -fno-PIE -fno-strict-aliasing -std=gnu11 -fms-extensions -Wno-gnu -Wno-microsoft-anon-tag -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -mno-sse4a -fcf-protection=branch -fno-jump-tables -m64 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mstack-alignment=8 -mskip-rax-setup -march=x86-64 -mtune=generic -mno-red-zone -mcmodel=kernel -mstack-protector-guard-reg=gs -mstack-protector-guard-symbol=__ref_stack_chk_guard -Wno-sign-compare -fno-asynchronous-unwind-tables -mretpoline-external-thunk -mindirect-branch-cs-prefix -mfunction-return=thunk-extern -mharden-sls=all -fpatchable-function-entry=16,16 -fno-delete-null-pointer-checks -O2 -fstack-protector-strong -ftrivial-auto-var-init=zero -fno-stack-clash-protection -pg -mfentry -DCC_USING_NOP_MCOUNT -DCC_USING_FENTRY -falign-functions=16 -fstrict-flex-arrays=3 -fno-strict-overflow -fno-stack-check -fno-builtin-wcslen -Wall -Wextra -Wundef -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Werror=strict-prototypes -Wno-format-security -Wno-trigraphs -Wno-frame-address -Wno-address-of-packed-member -Wmissing-declarations -Wmissing-prototypes -Wframe-larger-than=2048 -Wno-format-overflow-non-kprintf -Wno-format-truncation-non-kprintf -Wno-type-limits -Wno-pointer-sign -Wcast-function-type -Wimplicit-fallthrough -Werror=date-time -Werror=incompatible-pointer-types -Wenum-conversion -Wunused -Wno-unused-but-set-variable -Wno-unused-const-variable -Wno-format-overflow -Wno-override-init -Wno-pointer-to-enum-cast -Wno-tautological-constant-out-of-range-compare -Wno-unaligned-access -Wno-enum-compare-conditional -Wno-missing-field-initializers -Wno-shift-negative-value -Wno-enum-enum-conversion -Wno-sign-compare -Wno-unused-parameter -g -I././include    -fsanitize=kernel-address -mllvm -asan-mapping-offset=0xdffffc0000000000  -mllvm -asan-instrumentation-with-call-threshold=10000  -mllvm -asan-stack=0    -mllvm -asan-globals=1  -mllvm -asan-kernel-mem-intrinsic-prefix=1  -DMODULE  -DKBUILD_BASENAME='"stcp_rust_ffi"' -DKBUILD_MODNAME='"stcp"' -D__KBUILD_MODNAME=stcp -c -o src/stcp_rust_ffi.o src/stcp_rust_ffi.c  

source_src/stcp_rust_ffi.o := src/stcp_rust_ffi.c

deps_src/stcp_rust_ffi.o := \
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
  include/stcp_rust_ffi.h \
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

src/stcp_rust_ffi.o: $(deps_src/stcp_rust_ffi.o)

$(deps_src/stcp_rust_ffi.o):
