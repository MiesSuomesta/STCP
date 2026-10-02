/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * This header exists to force full rebuild when the compiler is upgraded.
 *
 * When fixdep scans this, it will find this string "CONFIG_CC_VERSION_TEXT"
 * and add dependency on include/config/CC_VERSION_TEXT, which is touched
 * by Kconfig when the version string from the compiler changes.
 */
/* Additional tree-wide dependencies start here. */
/*
 * If any of the GCC plugins change, we need to rebuild everything that
 * was built with them, as they may have changed their behavior and those
 * behaviors may need to be synchronized across all translation units.
 */
/*
 * If the randstruct seed itself changes (whether for GCC plugins or
 * Clang), the entire tree needs to be rebuilt since the randomization of
 * structures may change between compilation units if not.
 */
/*
 * If any external changes affect Clang's integer wrapping sanitizer
 * behavior, a full rebuild is needed as the coverage for wrapping types
 * may have changed, which may impact the expected behaviors that should
 * not differ between compilation units.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Automatically generated file; DO NOT EDIT.
 * Linux/x86 7.2.0-rc6 Kernel Configuration
 */
/*
 * The use of "&&" / "||" is limited in certain expressions.
 * The following enable to calculate "and" / "or" with macro expansion only.
 */
/*
 * Helper macros to use CONFIG_ options in C/CPP expressions. Note that
 * these only work with boolean and tristate options.
 */
/*
 * Getting something that works in C and CPP for an arg that may or may
 * not be defined is tricky.  Here, if we have "#define CONFIG_BOOGER 1"
 * we match on the placeholder define, insert the "0," for arg1 and generate
 * the triplet (0, 1, 0).  Then the last step cherry picks the 2nd arg (a one).
 * When CONFIG_BOOGER is not defined, we generate a (... 1, 0) pair, and when
 * the last step cherry picks the 2nd arg, we get a zero.
 */
/*
 * IS_BUILTIN(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y', 0
 * otherwise. For boolean options, this is equivalent to
 * IS_ENABLED(CONFIG_FOO).
 */
/*
 * IS_MODULE(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'm', 0
 * otherwise.  CONFIG_FOO=m results in "#define CONFIG_FOO_MODULE 1" in
 * autoconf.h.
 */
/*
 * IS_REACHABLE(CONFIG_FOO) evaluates to 1 if the currently compiled
 * code can call a function defined in code compiled based on CONFIG_FOO.
 * This is similar to IS_ENABLED(), but returns false when invoked from
 * built-in code when CONFIG_FOO is set to 'm'.
 */
/*
 * IS_ENABLED(CONFIG_FOO) evaluates to 1 if CONFIG_FOO is set to 'y' or 'm',
 * 0 otherwise.  Note that CONFIG_FOO=y results in "#define CONFIG_FOO 1" in
 * autoconf.h, while CONFIG_FOO=m results in "#define CONFIG_FOO_MODULE 1".
 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Linker script for 32-bit vDSO.
 * We #include the file to define the layout details.
 *
 * This file defines the version script giving the user-exported symbols in
 * the DSO.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * int-ll64 is used everywhere now.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * asm-generic/int-ll64.h
 *
 * Integer declarations for architectures which use "long long"
 * for 64-bit types.
 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * asm-generic/int-ll64.h
 *
 * Integer declarations for architectures which use "long long"
 * for 64-bit types.
 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * FIXME: The check currently breaks x86-64 build, so it's
 * temporarily disabled. Please fix x86-64 and reenable
 */
/*
 * small_const_nbits(n) is true precisely when it is known at compile-time
 * that BITMAP_SIZE(n) is 1, i.e. 1 <= n <= BITS_PER_LONG. This allows
 * various bit/bitmap APIs to provide a fast inline implementation. Bitmaps
 * of size 0 are very rare, and a compile-time-known-size 0 is most likely
 * a sign of error. They will be handled correctly by the bit/bitmap APIs,
 * but using the out-of-line functions, so that the inline implementations
 * can unconditionally dereference the pointer(s).
 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* const.h: Macros for dealing with constants.  */
/* Some constant macros are used in both assembler and
 * C code.  Therefore we cannot annotate them always with
 * 'UL' and other type specifiers unilaterally.  We
 * use the following macros to deal with this.
 *
 * Similarly, _AT() will cast an expression with a type in C, but
 * leave it unchanged in asm.
 */
/*
 * Divide positive or negative dividend by positive or negative divisor
 * and round to closest integer. Result is undefined for negative
 * divisors if the dividend variable type is unsigned and for negative
 * dividends if the divisor variable type is unsigned.
 */

/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * AMD Memory Encryption Support
 *
 * Copyright (C) 2016 Advanced Micro Devices, Inc.
 *
 * Author: Tom Lendacky <thomas.lendacky@amd.com>
 */

/* SPDX-License-Identifier: GPL-2.0 */
/*
 * PAGE_SHIFT determines the page size.
 *
 * Note: This definition is required because PAGE_SHIFT is used
 * in several places throughout the codebase.
 */
/* Cast P*D_MASK to a signed type so that it is sign-extended if
   virtual addresses are 32-bits but physical addresses are larger
   (ie, 32-bit PAE). */
/* Physical address where kernel should be loaded. */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The index for the tss.ist[] array. The hardware limit is 7 entries.
 */
/*
 * Set __PAGE_OFFSET to the most negative possible address +
 * PGDIR_SIZE*17 (pgd slot 273).
 *
 * The gap is to allow a space for LDT remap for PTI (1 pgd slot) and space for
 * a hypervisor (16 slots). Choosing 16 slots for a hypervisor is arbitrary,
 * but it's what Xen requires.
 */
/* See Documentation/arch/x86/x86_64/mm.rst for a description of the memory map. */
/* This decides where the kernel will search for a free chunk of vm
 * space during mmap's.
 */
/*
 * In spite of the name, KERNEL_IMAGE_SIZE is a limit on the maximum virtual
 * address for the kernel image, rather than the limit on the size itself.
 * This can be at most 1 GiB, due to the fixmap living in the next 1 GiB (see
 * level2_kernel_pgt in arch/x86/kernel/head_64.S).
 *
 * On KASLR use 1 GiB by default, leaving 1 GiB for modules once the
 * page tables are fully set up.
 *
 * If KASLR is disabled we can shrink it to 0.5 GiB and increase the size
 * of the modules area to 1.5 GiB.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * __has_builtin is supported on gcc >= 10, clang >= 3 and icc >= 21.
 * In the meantime, to support gcc < 10, we implement __has_builtin
 * by hand.
 */
/* Indirect macros required for expanded argument pasting, eg. __LINE__. */
/*
 * The below symbols may be defined for one or more, but not ALL, of the above
 * compilers. We don't consider that to be an error, so set them to nothing.
 * For example, some of them are for compiler specific plugins.
 */
/*
 * Any place that could be marked with the "alloc_size" attribute is also
 * a place to be marked with the "malloc" attribute, except those that may
 * be performing a _reallocation_, as that may alias the existing pointer.
 * For these, use __realloc_size().
 */
/*
 * When the size of an allocated object is needed, use the best available
 * mechanism to find it. (For cases where sizeof() cannot be used.)
 *
 * Optional: only supported since gcc >= 12
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Object-Size-Checking.html
 * clang: https://clang.llvm.org/docs/LanguageExtensions.html#evaluating-object-size
 */
/*
 * Determine if an attribute has been applied to a variable.
 * Using __annotated needs to check for __annotated being available,
 * or negative tests may fail when annotation cannot be checked. For
 * example, see the definition of __is_cstr().
 */
/*
 * Optional: only supported since gcc >= 15, clang >= 19
 *
 *   gcc: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-_005f_005fbuiltin_005fcounted_005fby_005fref
 * clang: https://clang.llvm.org/docs/LanguageExtensions.html#builtin-counted-by-ref
 */
/*
 * Some versions of gcc do not mark 'asm goto' volatile:
 *
 *  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=103979
 *
 * We do it here by hand, because it doesn't hurt.
 */
/*
 * Clang has trouble with constraints with multiple
 * alternative behaviors ("g" , "rm" and "=rm").
 */
/*
 * __signed_scalar_typeof(x) - Declare a signed scalar type, leaving
 *			       non-scalar types unchanged.
 */
/* Is this type a native word size -- useful for atomic operations */
/**
 * compiletime_assert - break build and emit msg if condition is false
 * @condition: a compile-time constant condition to check
 * @msg:       a message to emit if condition is false
 *
 * In tradition of POSIX assert, this macro will break the build if the
 * supplied condition is *false*, emitting the supplied error message if the
 * compiler has support to do so.
 */
/* Helpers for emitting diagnostics in pragmas. */
/* Indirect stringification.  Doing two levels allows the parameter to be a
 * macro itself.  For example, compile with -DFOO=bar, __stringify(FOO)
 * converts to "bar".
 */
/* SPDX-License-Identifier: GPL-2.0-only */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Force the compiler to emit 'sym' as a symbol, so that we can reference
 * it from inline assembler. Necessary in case 'sym' could be inlined
 * otherwise, or eliminated entirely due to lack of references that are
 * visible to the compiler.
 */
/*
 * This returns a constant expression while determining if an argument is
 * a constant expression, most importantly without evaluating the argument.
 * Glory to Martin Uecker <Martin.Uecker@med.uni-goettingen.de>
 *
 * Details:
 * - sizeof() return an integer constant expression, and does not evaluate
 *   the value of its operand; it only examines the type of its operand.
 * - The results of comparing two integer constant expressions is also
 *   an integer constant expression.
 * - The first literal "8" isn't important. It could be any literal value.
 * - The second literal "8" is to avoid warnings about unaligned pointers;
 *   this could otherwise just be "1".
 * - (long)(x) is used to avoid warnings about 64-bit types on 32-bit
 *   architectures.
 * - The C Standard defines "null pointer constant", "(void *)0", as
 *   distinct from other void pointers.
 * - If (x) is an integer constant expression, then the "* 0l" resolves
 *   it into an integer constant expression of value 0. Since it is cast to
 *   "void *", this makes the second operand a null pointer constant.
 * - If (x) is not an integer constant expression, then the second operand
 *   resolves to a void pointer (but not a null pointer constant: the value
 *   is not an integer constant 0).
 * - The conditional operator's third operand, "(int *)8", is an object
 *   pointer (to type "int").
 * - The behavior (including the return type) of the conditional operator
 *   ("operand1 ? operand2 : operand3") depends on the kind of expressions
 *   given for the second and third operands. This is the central mechanism
 *   of the macro:
 *   - When one operand is a null pointer constant (i.e. when x is an integer
 *     constant expression) and the other is an object pointer (i.e. our
 *     third operand), the conditional operator returns the type of the
 *     object pointer operand (i.e. "int *"). Here, within the sizeof(), we
 *     would then get:
 *       sizeof(*((int *)(...))  == sizeof(int)  == 4
 *   - When one operand is a void pointer (i.e. when x is not an integer
 *     constant expression) and the other is an object pointer (i.e. our
 *     third operand), the conditional operator returns a "void *" type.
 *     Here, within the sizeof(), we would then get:
 *       sizeof(*((void *)(...)) == sizeof(void) == 1
 * - The equality comparison to "sizeof(int)" therefore depends on (x):
 *     sizeof(int) == sizeof(int)     (x) was a constant expression
 *     sizeof(int) != sizeof(void)    (x) was not a constant expression
 */
/*
 * Whether 'type' is a signed type or an unsigned type. Supports scalar types,
 * bool and also pointer types.
 */
/*
 * Useful shorthand for "is this condition known at compile-time?"
 *
 * Note that the condition may involve non-constant values,
 * but the compiler may know enough about the details of the
 * values to determine that the condition is statically true.
 */
/*
 * Similar to statically_true() but produces a constant expression
 *
 * To be used in conjunction with macros, such as BUILD_BUG_ON_ZERO(),
 * which require their input to be a constant expression and for which
 * statically_true() would otherwise fail.
 *
 * This is a trade-off: const_true() requires all its operands to be
 * compile time constants. Else, it would always returns false even on
 * the most trivial cases like:
 *
 *   true || non_const_var
 *
 * On the opposite, statically_true() is able to fold more complex
 * tautologies and will return true on expressions such as:
 *
 *   !(non_const_var * 8 % 4)
 *
 * For the general case, statically_true() is better.
 */
/*
 * This is needed in functions which generate the stack canary, see
 * arch/x86/kernel/smpboot.c::start_secondary() for an example.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Prevent the compiler from merging or refetching reads or writes. The
 * compiler is also forbidden from reordering successive instances of
 * READ_ONCE and WRITE_ONCE, but only when the compiler is aware of some
 * particular ordering. One way to make the compiler aware of ordering is to
 * put the two invocations of READ_ONCE or WRITE_ONCE in different C
 * statements.
 *
 * These two macros will also work on aggregate data types like structs or
 * unions.
 *
 * Their two major use cases are: (1) Mediating communication between
 * process-level code and irq/NMI handlers, all running on the same CPU,
 * and (2) Ensuring that the compiler does not fold, spindle, or otherwise
 * mutilate accesses that either do not require ordering or that interact
 * with an explicit memory barrier or atomic instruction that provides the
 * required ordering.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This comment block is used by fixdep. Please do not remove.
 *
 * When CONFIG_MODVERSIONS is changed from n to y, all source files having
 * EXPORT_SYMBOL variants must be re-compiled because genksyms is run as a
 * side effect of the *.o build rule.
 */
/*
 * LLVM integrated assembler cam merge adjacent string literals (like
 * C and GNU-as) passed to '.ascii', but not to '.asciz' and chokes on:
 *
 *   .asciz "MODULE_" "kvm" ;
 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * The rules for enabling IBT are:
 *
 *  - CC_HAS_IBT:         the toolchain supports it
 *  - X86_KERNEL_IBT:     it is selected in Kconfig
 *  - !__DISABLE_EXPORTS: this is regular kernel code
 *
 * Esp. that latter one is a bit non-obvious, but some code like compressed,
 * purgatory, realmode etc.. is built with custom CFLAGS that do not include
 * -fcf-protection=branch and things will go *bang*.
 *
 * When all the above are satisfied, HAS_KERNEL_IBT will be 1, otherwise 0.
 */
/*
 * The generic version tends to create spurious ENDBR instructions under
 * certain conditions.
 */
/*
 * Depending on -fpatchable-function-entry=N,N usage (CONFIG_CALL_PADDING) the
 * CFI symbol layout changes.
 *
 * Without CALL_PADDING:
 *
 * 	.align	FUNCTION_ALIGNMENT
 * __cfi_##name:
 * 	.skip	FUNCTION_PADDING, 0x90
 * 	.byte   0xb8
 * 	.long	__kcfi_typeid_##name
 * name:
 *
 * With CALL_PADDING:
 *
 * 	.align FUNCTION_ALIGNMENT
 * __cfi_##name:
 * 	.byte	0xb8
 * 	.long	__kcfi_typeid_##name
 * 	.skip	FUNCTION_PADDING, 0x90
 * name:
 *
 * In both cases the whole thing is FUNCTION_ALIGNMENT aligned and sized.
 */
/* UML needs to be able to override memcpy() and friends for KASAN. */
/* SYM_TYPED_FUNC_START -- use for indirectly called globals, w/ CFI type */
/* SYM_FUNC_START -- use for global functions */
/* SYM_FUNC_START_NOALIGN -- use for global functions, w/o alignment */
/* SYM_FUNC_START_LOCAL -- use for local functions */
/* SYM_FUNC_START_LOCAL_NOALIGN -- use for local functions, w/o alignment */
/* SYM_FUNC_START_WEAK -- use for weak functions */
/* SYM_FUNC_START_WEAK_NOALIGN -- use for weak functions, w/o alignment */
/*
 * Expose 'sym' to the startup code in arch/x86/boot/startup/, by emitting an
 * alias prefixed with __pi_
 */

/* Some toolchains use other characters (e.g. '`') to mark new line in macro */
/*
 * For assembly routines.
 *
 * Note when using these that you must specify the appropriate
 * alignment directives yourself
 */
/*
 * This is used by architectures to keep arguments on the stack
 * untouched by the compiler by keeping them live until the end.
 * The argument stack may be owned by the assembly-language
 * caller, not the callee, and gcc doesn't always understand
 * that.
 *
 * We have the return value, and a maximum of six arguments.
 *
 * This should always be followed by a "return ret" for the
 * protection to work (ie no more work that the compiler might
 * end up needing stack temporaries for).
 */
/* Assembly files may be compiled with -traditional .. */
/* SYM_T_FUNC -- type used by assembler to mark functions */
/* SYM_T_OBJECT -- type used by assembler to mark data */
/* SYM_T_NONE -- type used by assembler to mark entries of unknown type */
/* SYM_A_* -- align the symbol? */
/* SYM_L_* -- linkage of symbols */
/* === generic annotations === */
/* SYM_ENTRY -- use only if you have to for non-paired symbols */
/* SYM_START -- use only if you have to */
/* SYM_END -- use only if you have to */
/* SYM_ALIAS -- use only if you have to */
/* === code annotations === */
/*
 * FUNC -- C-like functions (proper stack frame etc.)
 * CODE -- non-C code (e.g. irq handlers with different, special stack etc.)
 *
 * Objtool validates stack for FUNC, but not for CODE.
 * Objtool generates debug info for both FUNC & CODE, but needs special
 * annotations for each CODE's start (to describe the actual stack frame).
 *
 * Objtool requires that all code must be contained in an ELF symbol. Symbol
 * names that have a  .L prefix do not emit symbol table entries. .L
 * prefixed symbols can be used within a code region, but should be avoided for
 * denoting a range of code via ``SYM_*_START/END`` annotations.
 *
 * ALIAS -- does not generate debug info -- the aliased function will
 */
/* SYM_INNER_LABEL_ALIGN -- only for labels in the middle of code */
/* SYM_INNER_LABEL -- only for labels in the middle of code */
/* SYM_FUNC_START -- use for global functions */
/* SYM_FUNC_START_NOALIGN -- use for global functions, w/o alignment */
/* SYM_FUNC_START_LOCAL -- use for local functions */
/* SYM_FUNC_START_LOCAL_NOALIGN -- use for local functions, w/o alignment */
/* SYM_FUNC_START_WEAK -- use for weak functions */
/* SYM_FUNC_START_WEAK_NOALIGN -- use for weak functions, w/o alignment */
/*
 * SYM_FUNC_END -- the end of SYM_FUNC_START_LOCAL, SYM_FUNC_START,
 * SYM_FUNC_START_WEAK, ...
 */
/*
 * SYM_FUNC_ALIAS -- define a global alias for an existing function
 */
/*
 * SYM_FUNC_ALIAS_LOCAL -- define a local alias for an existing function
 */
/*
 * SYM_FUNC_ALIAS_WEAK -- define a weak global alias for an existing function
 */
/* SYM_CODE_START -- use for non-C (special) functions */
/* SYM_CODE_START_NOALIGN -- use for non-C (special) functions, w/o alignment */
/* SYM_CODE_START_LOCAL -- use for local non-C (special) functions */
/*
 * SYM_CODE_START_LOCAL_NOALIGN -- use for local non-C (special) functions,
 * w/o alignment
 */
/* SYM_CODE_END -- the end of SYM_CODE_START_LOCAL, SYM_CODE_START, ... */
/* === data annotations === */
/* SYM_DATA_START -- global data symbol */
/* SYM_DATA_START -- local data symbol */
/* SYM_DATA_END -- the end of SYM_DATA_START symbol */
/* SYM_DATA_END_LABEL -- the labeled end of SYM_DATA_START symbol */
/* SYM_DATA -- start+end wrapper around simple global data */
/* SYM_DATA_LOCAL -- start+end wrapper around simple local data */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type int), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma expressions
 * aren't permitted).
 *
 * Take an error message as an optional second argument. If omitted,
 * default to the stringification of the tested expression.
 */
/* Force a compilation error if a constant expression is not a power of 2 */
/*
 * BUILD_BUG_ON_INVALID() permits the compiler to check the validity of the
 * expression but avoids the generation of any code, even if that expression
 * has side-effects.
 */
/**
 * BUILD_BUG_ON_MSG - break compile if a condition is true & emit supplied
 *		      error message.
 * @cond: the condition which the compiler should know is false.
 * @msg: build-time error message
 *
 * See BUILD_BUG_ON for description.
 */
/**
 * BUILD_BUG_ON - break compile if a condition is true.
 * @condition: the condition which the compiler should know is false.
 *
 * If you have some code which relies on certain constants being equal, or
 * some other compile-time-evaluated condition, you should use BUILD_BUG_ON to
 * detect if someone changes it.
 */
/**
 * BUILD_BUG - break compile if used.
 *
 * If you have some code that you expect the compiler to eliminate at
 * build time, you should use BUILD_BUG to detect if it is
 * unexpectedly used.
 */
/**
 * static_assert - check integer constant expression at build time
 * @expr: expression to be checked
 *
 * static_assert() is a wrapper for the C11 _Static_assert, with a
 * little macro magic to make the message optional (defaulting to the
 * stringification of the tested expression).
 *
 * Contrary to BUILD_BUG_ON(), static_assert() can be used at global
 * scope, but requires the expression to be an integer constant
 * expression (i.e., it is not enough that __builtin_constant_p() is
 * true for expr).
 *
 * Also note that BUILD_BUG_ON() fails the build if the condition is
 * true, while static_assert() fails the build if the expression is
 * false.
 */
/*
 * Compile time check that field has an expected offset
 */
/* These macros are used to mark some functions or 
 * initialized data (doesn't apply to uninitialized data)
 * as `initialization' functions. The kernel can take this
 * as hint that the function is used only during the initialization
 * phase and free up used memory resources after
 *
 * Usage:
 * For functions:
 * 
 * You should add __init immediately before the function name, like:
 *
 * static void __init initme(int x, int y)
 * {
 *    extern int z; z = x * y;
 * }
 *
 * If the function has a prototype somewhere, you can also add
 * __init between closing brace of the prototype and semicolon:
 *
 * extern int initialize_foobar_device(int, int, int) __init;
 *
 * For initialized data:
 * You should insert __initdata or __initconst between the variable name
 * and equal sign followed by value, e.g.:
 *
 * static int init_variable __initdata = 0;
 * static const char linux_logo[] __initconst = { 0x32, 0x36, ... };
 *
 * Don't forget to initialize data not at file scope, i.e. within a function,
 * as gcc otherwise puts the data into the bss section and not into the init
 * section.
 */
/* These are for everybody (although not all archs will actually
   discard it in modules) */
/*
 * modpost check for section mismatches during the kernel build.
 * A section mismatch happens when there are references from a
 * code or data section to an init section (both code or data).
 * The init sections are (for most archs) discarded by the kernel
 * when early init has completed so all such references are potential bugs.
 * For exit sections the same issue exists.
 *
 * The following markers are used for the cases where the reference to
 * the *init / *exit section (code or data) is valid and will teach
 * modpost not to issue a warning.  Intended semantics is that a code or
 * data tagged __ref* can reference code or data from init section without
 * producing a warning (of course, no warning does not mean code is
 * correct, so optimally document why the __ref is needed and why it's OK).
 *
 * The markers follow same syntax rules as __init / __initdata.
 */
/* For assembly routines */
/* silence warnings when references are OK */
/* Data marked not to be saved by software suspend */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 */

/*
 * Linker script for vDSO.  This is an ELF shared object prelinked to
 * its virtual address, and with only one read-only segment.
 * This script controls its layout.
 */
SECTIONS
{
 /*
	 * User/kernel shared data is before the vDSO.  This may be a little
	 * uglier than putting it after the vDSO, but it avoids issues with
	 * non-allocatable things that dangle past the end of the PT_LOAD
	 * segment.
	 */
 PROVIDE(vdso_u_data = . - 6 * (1 << 12)); PROVIDE(vdso_u_time_data = vdso_u_data); PROVIDE(vdso_u_rng_data = vdso_u_data + 2 * (1 << 12));
 vclock_pages = ((vdso_u_data) + (6 - 2) * (1 << 12));
 pvclock_page = vclock_pages + 0 * (1 << 12);
 hvclock_page = vclock_pages + 1 * (1 << 12);
 . = SIZEOF_HEADERS;
 .hash : { *(.hash) } :text
 .gnu.hash : { *(.gnu.hash) }
 .dynsym : { *(.dynsym) }
 .dynstr : { *(.dynstr) }
 .gnu.version : { *(.gnu.version) }
 .gnu.version_d : { *(.gnu.version_d) }
 .gnu.version_r : { *(.gnu.version_r) }
 .dynamic : { *(.dynamic) } :text :dynamic
 .rodata : {
  *(.rodata*)
  *(.data*)
  *(.sdata*)
  *(.got.plt) *(.got)
  *(.gnu.linkonce.d.*)
  *(.bss*)
  *(.dynbss*)
  *(.gnu.linkonce.b.*)
 } :text
 .note.gnu.property : {
  *(.note.gnu.property)
 } :text :note :gnu_property
 .note : {
  *(.note*)
 } :text :note
 .eh_frame_hdr : { *(.eh_frame_hdr) } :text :eh_frame_hdr
 .eh_frame : {
  KEEP (*(.eh_frame))
  *(.eh_frame.*)
 } :text
 /*
	 * Text is well-separated from actual data: there's plenty of
	 * stuff that isn't used at runtime in between.
	 */
 .text : {
  *(.text*)
 } :text =0x90909090,
 .altinstructions : { *(.altinstructions) } :text
 .altinstr_replacement : { *(.altinstr_replacement) } :text
 __ex_table : { *(__ex_table) } :text
 /DISCARD/ : {
  *(.discard)
  *(.discard.*)
  *(__bug_table)
 }
}
/*
 * Very old versions of ld do not recognize this name token; use the constant.
 */
/*
 * We must supply the ELF program headers explicitly to get just one
 * PT_LOAD segment, and set the flags explicitly to make segments read-only.
*/
PHDRS
{
 text PT_LOAD FLAGS(5) FILEHDR PHDRS;
 dynamic PT_DYNAMIC FLAGS(4);
 note PT_NOTE FLAGS(4);
 eh_frame_hdr 0x6474e550 FLAGS(4);
 gnu_stack 0x6474e551 FLAGS(6);
 gnu_property 0x6474e553 FLAGS(4);
}

/* The ELF entry point can be used to set the AT_SYSINFO value.  */
ENTRY(__kernel_vsyscall);
/*
 * This controls what userland symbols we export from the vDSO.
 */
VERSION
{
 LINUX_2.6 {
 global:
  __vdso_clock_gettime;
  __vdso_gettimeofday;
  __vdso_time;
  __vdso_clock_getres;
  __vdso_clock_gettime64;
  __vdso_clock_getres_time64;
  __vdso_getcpu;
  __vdso_futex_robust_list32_try_unlock;
 };
 LINUX_2.5 {
 global:
  __kernel_vsyscall;
  __kernel_sigreturn;
  __kernel_rt_sigreturn;
 local: *;
 };
}
