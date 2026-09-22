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
 * Linux/arm64 6.18.44 Kernel Configuration
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
/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * GNU linker script for the VDSO library.
*
 * Copyright (C) 2012 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 * Heavily based on the vDSO linker scripts for other archs.
 */
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
 * Based on arch/arm/include/asm/page.h
 *
 * Copyright (C) 1995-2003 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */
/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Based on arch/arm/include/asm/page.h
 *
 * Copyright (C) 1995-2003 Russell King
 * Copyright (C) 2017 ARM Ltd.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/* const.h: Macros for dealing with constants.  */
/*
 * PAGE_SHIFT determines the page size.
 *
 * Note: This definition is required because PAGE_SHIFT is used
 * in several places throughout the codebase.
 */
/* SPDX-License-Identifier: GPL-2.0 */
/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2012 ARM Limited
 */
/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Helper macros to support writing architecture specific
 * linker scripts.
 *
 * A minimal linker scripts has following content:
 * [This is a sample, architectures may have special requirements]
 *
 * OUTPUT_FORMAT(...)
 * OUTPUT_ARCH(...)
 * ENTRY(...)
 * SECTIONS
 * {
 *	. = START;
 *	__init_begin = .;
 *	HEAD_TEXT_SECTION
 *	INIT_TEXT_SECTION(PAGE_SIZE)
 *	INIT_DATA_SECTION(...)
 *	PERCPU_SECTION(CACHELINE_SIZE)
 *	__init_end = .;
 *
 *	_stext = .;
 *	TEXT_SECTION = 0
 *	_etext = .;
 *
 *      _sdata = .;
 *	RO_DATA(PAGE_SIZE)
 *	RW_DATA(...)
 *	_edata = .;
 *
 *	EXCEPTION_TABLE(...)
 *
 *	BSS_SECTION(0, 0, 0)
 *	_end = .;
 *
 *	STABS_DEBUG
 *	DWARF_DEBUG
 *	ELF_DETAILS
 *
 *	DISCARDS		// must be the last
 * }
 *
 * [__init_begin, __init_end] is the init section that may be freed after init
 * 	// __init_begin and __init_end should be page aligned, so that we can
 *	// free the whole .init memory
 * [_stext, _etext] is the text section
 * [_sdata, _edata] is the data section
 *
 * Some of the included output section have their own set of constants.
 * Examples are: [__initramfs_start, __initramfs_end] for initramfs and
 *               [__nosave_begin, __nosave_end] for the nosave data
 */
/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * For codetags which might be used after module unload, therefore might stay
 * longer in memory. Each such codetag type has its own section so that we can
 * unload them individually once unused.
 */
/*
 * Only some architectures want to have the .notes segment visible in
 * a separate PT_NOTE ELF Program Header. When this happens, it needs
 * to be visible in both the kernel text's PT_LOAD and the PT_NOTE
 * Program Headers. In this case, though, the PT_LOAD needs to be made
 * the default again so that all the following sections don't also end
 * up in the PT_NOTE Program Header.
 */
/*
 * Some architectures have non-executable read-only exception tables.
 * They can be added to the RO_DATA segment by specifying their desired
 * alignment.
 */
/* Align . function alignment. */
/*
 * LD_DEAD_CODE_DATA_ELIMINATION option enables -fdata-sections, which
 * generates .data.identifier sections, which need to be pulled in with
 * .data. We don't want to pull in .data..other sections, which Linux
 * has defined. Same for text and bss.
 *
 * With LTO_CLANG, the linker also splits sections by default, so we need
 * these macros to combine the sections during the final link.
 *
 * With AUTOFDO_CLANG and PROPELLER_CLANG, by default, the linker splits
 * text sections and regroups functions into subsections.
 *
 * RODATA_MAIN is not used because existing code already defines .rodata.x
 * sections to be brought in with rodata.
 */
/*
 * GCC 4.5 and later have a 32 bytes section alignment for structures.
 * Except GCC 4.9, that feels the need to align on 64 bytes.
 */
/*
 * The order of the sched class addresses are important, as they are
 * used to determine the order of the priority of each sched class in
 * relation to each other.
 */
/* The actual configuration determine if the init/exit sections
 * are handled as text/data or they can be discarded (which
 * often happens at runtime)
 */
/*
 * The ftrace call sites are logged to a section whose name depends on the
 * compiler option used. A given kernel image will only use one, AKA
 * FTRACE_CALLSITE_SECTION. We capture all of them here to avoid header
 * dependencies for FTRACE_CALLSITE_SECTION's definition.
 *
 * ftrace_ops_list_func will be defined as arch_ftrace_ops_list_func
 * as some archs will have a different prototype for that function
 * but ftrace_ops_list_func() will have a single prototype.
 */
/*
 * .data section
 */
/*
 * Data section helpers
 */
/*
 * Allow architectures to handle ro_after_init data on their
 * own by defining an empty RO_AFTER_INIT_DATA.
 */
/*
 * .kcfi_traps contains a list KCFI trap locations.
 */
/*
 * Read only Data
 */
/*
 * Non-instrumentable text section
 */
/*
 * .text section. Map to function alignment to avoid address changes
 * during second ld run in second ld pass when generating System.map
 *
 * TEXT_MAIN here will match symbols with a fixed pattern (for example,
 * .text.hot or .text.unlikely) if dead code elimination or
 * function-section is enabled. Match these symbols first before
 * TEXT_MAIN to ensure they are grouped together.
 *
 * Also placing .text.hot section at the beginning of a page, this
 * would help the TLB performance.
 */
/* sched.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* spinlock.text is aling to function alignment to secure we have same
 * address even at second ld pass when generating System.map */
/* Section used for early init (in .S files) */
/*
 * Exception table
 */
/*
 * .BTF
 */
/*
 * Init task
 */
/* init and exit section handling */
/*
 * bss (Block Started by Symbol) - uninitialized data
 * zeroed during startup
 */
/*
 * Allow archectures to redefine BSS_FIRST_SECTIONS to add extra
 * sections to the front of bss.
 */
/*
 * DWARF debug sections.
 * Symbols in the DWARF debugging sections are relative to
 * the beginning of the section so we begin them at 0.
 */
/* Stabs debugging sections. */
/* Required sections not related to debugging. */
/* Built-in firmware blobs */
/*
 * Discard .note.GNU-stack, which is emitted as PROGBITS by the compiler.
 * Otherwise, the type of .notes section would become PROGBITS instead of NOTES.
 *
 * Also, discard .note.gnu.property, otherwise it forces the notes section to
 * be 8-byte aligned which causes alignment mismatches with the kernel's custom
 * 4-byte aligned notes.
 */
/* Alignment must be consistent with (kunit_suite *) in include/kunit/test.h */
/* Alignment must be consistent with (kunit_suite *) in include/kunit/test.h */
/*
 * Memory encryption operates on a page basis. Since we need to clear
 * the memory encryption mask for this section, it needs to be aligned
 * on a page boundary and be a page-size multiple in length.
 *
 * Note: We use a separate section so that only this section gets
 * decrypted to avoid exposing more than we wish.
 */
/*
 * Default discarded sections.
 *
 * Some archs want to discard exit text/data at runtime rather than
 * link time due to cross-section references such as alt instructions,
 * bug table, eh_frame, etc.  DISCARDS must be the last of output
 * section definitions so that such archs put those in earlier section
 * definitions.
 */
/*
 * Clang's -fprofile-arcs, -fsanitize=kernel-address, and
 * -fsanitize=thread produce unwanted sections (.eh_frame
 * and .init_array.*), but CONFIG_CONSTRUCTORS wants to
 * keep any .init_array.* sections.
 * https://llvm.org/pr46478
 */
/**
 * PERCPU_INPUT - the percpu input sections
 * @cacheline: cacheline size
 *
 * The core percpu section names and core symbols which do not rely
 * directly upon load addresses.
 *
 * @cacheline is used to align subsections to avoid false cacheline
 * sharing between subsections for different purposes.
 */
/**
 * PERCPU_SECTION - define output section for percpu area
 * @cacheline: cacheline size
 *
 * Macro which expands to output section for percpu area.
 *
 * @cacheline is used to align subsections to avoid false cacheline
 * sharing between subsections for different purposes.
 */
/*
 * Definition of the high level *_SECTION macros
 * They will fit only a subset of the architectures
 */
/*
 * Writeable data.
 * All sections are combined in a single .data section.
 * The sections following CONSTRUCTORS are arranged so their
 * typical alignment matches.
 * A cacheline is typical/always less than a PAGE_SIZE so
 * the sections that has this restriction (or similar)
 * is located before the ones requiring PAGE_SIZE alignment.
 * NOSAVE_DATA starts and ends with a PAGE_SIZE alignment which
 * matches the requirement of PAGE_ALIGNED_DATA.
 *
 * use 0 as page_align if page_aligned data is not used */
/* SPDX-License-Identifier: GPL-2.0 */
OUTPUT_FORMAT("elf64-littleaarch64", "elf64-bigaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
SECTIONS
{
 PROVIDE(vdso_u_data = . - 4 * (1 << 12)); PROVIDE(vdso_u_time_data = vdso_u_data); PROVIDE(vdso_u_rng_data = vdso_u_data + 2 * (1 << 12));
 . = SIZEOF_HEADERS;
 .hash : { *(.hash) } :text
 .gnu.hash : { *(.gnu.hash) }
 .dynsym : { *(.dynsym) }
 .dynstr : { *(.dynstr) }
 .gnu.version : { *(.gnu.version) }
 .gnu.version_d : { *(.gnu.version_d) }
 .gnu.version_r : { *(.gnu.version_r) }
 /*
	 * Discard .note.gnu.property sections which are unused and have
	 * different alignment requirement from vDSO note sections.
	 */
 /DISCARD/ : {
  *(.note.GNU-stack .note.gnu.property)
  *(.ARM.attributes)
 }
 .note : { *(.note.*) } :text :note
 . = ALIGN(16);
 .text : { *(.text*) } :text =0xd503201f
 PROVIDE (__etext = .);
 PROVIDE (_etext = .);
 PROVIDE (etext = .);
 . = ALIGN(4);
 .altinstructions : {
  *(.altinstructions)
 }
 .dynamic : { *(.dynamic) } :text :dynamic
 .rela.dyn : ALIGN(8) { *(.rela .rela*) }
 .rodata : {
  *(.rodata*)
  *(.got)
  *(.got.plt)
  *(.plt)
  *(.plt.*)
  *(.iplt)
  *(.igot .igot.plt)
 } :text
 _end = .;
 PROVIDE(end = .);
 .debug 0 : { *(.debug) } .line 0 : { *(.line) } .debug_srcinfo 0 : { *(.debug_srcinfo) } .debug_sfnames 0 : { *(.debug_sfnames) } .debug_aranges 0 : { *(.debug_aranges) } .debug_pubnames 0 : { *(.debug_pubnames) } .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) } .debug_abbrev 0 : { *(.debug_abbrev) } .debug_line 0 : { *(.debug_line) } .debug_frame 0 : { *(.debug_frame) } .debug_str 0 : { *(.debug_str) } .debug_loc 0 : { *(.debug_loc) } .debug_macinfo 0 : { *(.debug_macinfo) } .debug_pubtypes 0 : { *(.debug_pubtypes) } .debug_ranges 0 : { *(.debug_ranges) } .debug_weaknames 0 : { *(.debug_weaknames) } .debug_funcnames 0 : { *(.debug_funcnames) } .debug_typenames 0 : { *(.debug_typenames) } .debug_varnames 0 : { *(.debug_varnames) } .debug_gnu_pubnames 0 : { *(.debug_gnu_pubnames) } .debug_gnu_pubtypes 0 : { *(.debug_gnu_pubtypes) } .debug_types 0 : { *(.debug_types) } .debug_addr 0 : { *(.debug_addr) } .debug_line_str 0 : { *(.debug_line_str) } .debug_loclists 0 : { *(.debug_loclists) } .debug_macro 0 : { *(.debug_macro) } .debug_names 0 : { *(.debug_names) } .debug_rnglists 0 : { *(.debug_rnglists) } .debug_str_offsets 0 : { *(.debug_str_offsets) }
 .comment 0 : { *(.comment) } .symtab 0 : { *(.symtab) } .strtab 0 : { *(.strtab) } .shstrtab 0 : { *(.shstrtab) }
 /DISCARD/ : {
  *(.data .data.* .gnu.linkonce.d.* .sdata*)
  *(.bss .sbss .dynbss .dynsbss)
  *(.eh_frame .eh_frame_hdr)
 }
}
/*
 * We must supply the ELF program headers explicitly to get just one
 * PT_LOAD segment, and set the flags explicitly to make segments read-only.
 */
PHDRS
{
 text PT_LOAD FLAGS(5) FILEHDR PHDRS; /* PF_R|PF_X */
 dynamic PT_DYNAMIC FLAGS(4); /* PF_R */
 note PT_NOTE FLAGS(4); /* PF_R */
}
/*
 * This controls what symbols we export from the DSO.
 */
VERSION
{
 LINUX_2.6.39 {
 global:
  __kernel_rt_sigreturn;
  __kernel_gettimeofday;
  __kernel_clock_gettime;
  __kernel_clock_getres;
  __kernel_getrandom;
 local: *;
 };
}
/*
 * Make the sigreturn code visible to the kernel.
 */
VDSO_sigtramp = __kernel_rt_sigreturn;
