


OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(phys_startup_64)
jiffies = jiffies_64;
const_current_task = current_task;
const_cpu_current_top_of_stack = cpu_current_top_of_stack;
ASSERT(__relocate_kernel_end - __relocate_kernel_start <= 2048,
 "relocate_kernel code too large!")
PHDRS {
 text PT_LOAD FLAGS(5);
 data PT_LOAD FLAGS(6);
 note PT_NOTE FLAGS(0);
}
SECTIONS
{
 . = (0xffffffff80000000 + (((0x1000000) + (0x200000 - 1)) & ~(0x200000 - 1)));
 phys_startup_64 = ABSOLUTE(startup_64 - 0xffffffff80000000);
 .text : AT(ADDR(.text) - 0xffffffff80000000) {
  _text = .;
  __pi__text = .;
  _stext = .;
  . = ALIGN((1 << 21));
  *(.text..__x86.rethunk_untrain)
  . = ALIGN(16); __entry_text_start = .; *(.entry.text) __entry_text_end = .;
  . = srso_alias_untrain_ret | (1 << 2) | (1 << 8) | (1 << 14) | (1 << 20);
  *(.text..__x86.rethunk_safe)
  . = ALIGN((1 << 21));
  . = ALIGN(16); *(.text.asan.* .text.tsan.*) *(.text.unknown .text.unknown.*) __split_text_start = .; *(.text.split .text.split.[0-9a-zA-Z_]*) __split_text_end = .; __unlikely_text_start = .; *(.text.unlikely .text.unlikely.*) __unlikely_text_end = .; . = ALIGN((1 << 12)); __hot_text_start = .; *(.text.hot .text.hot.*) __hot_text_end = .; *(.text .text.[_0-9A-Za-df-rt-z]* .text.s[_0-9A-Za-su-z]* .text.s .text.s.* .text.st[_0-9A-Zb-z]* .text.st .text.st.* .text.sta[_0-9A-Za-qs-z]* .text.sta .text.sta.* .text.star[_0-9A-Za-su-z]* .text.star .text.star.* .text.start[_0-9A-Za-tv-z]* .text.start .text.start.* .text.startu[_0-9A-Za-oq-z]* .text.startu .text.startu.* .text.startup[_0-9A-Za-z]* .text.e[_0-9A-Za-wy-z]* .text.e .text.e.* .text.ex[_0-9A-Za-hj-z]* .text.ex .text.ex.* .text.exi[_0-9A-Za-su-z]* .text.exi .text.exi.* .text.exit[_0-9A-Za-z]* .text.fixup) . = ALIGN(16); __noinstr_text_start = .; *(.noinstr.text) __cpuidle_text_start = .; *(.cpuidle.text) __cpuidle_text_end = .; __noinstr_text_end = .; *(.ref.text)
  . = ALIGN(16); __sched_text_start = .; *(.sched.text) __sched_text_end = .;
  . = ALIGN(16); __lock_text_start = .; *(.spinlock.text) __lock_text_end = .;
  . = ALIGN(16); __kprobes_text_start = .; *(.kprobes.text) __kprobes_text_end = .;
  . = ALIGN(16); __softirqentry_text_start = .; *(.softirqentry.text) __softirqentry_text_end = .;
  *(.text..__x86.indirect_thunk)
  *(.text..__x86.return_thunk)
  . = ALIGN(16); __static_call_text_start = .; *(.static_call.text) __static_call_text_end = .;
  *(.gnu.warning)
 } :text = 0xcccccccc
 _etext = .;
 . = ALIGN((1 << 12));
 . = ALIGN((1 << 21));
 . = ALIGN(((1 << 12))); .rodata : AT(ADDR(.rodata) - 0xffffffff80000000) { __start_rodata = .; *(.rodata) *(.rodata.*) *(.data.rel.ro*) . = ALIGN(32); __sched_class_highest = .; *(__stop_sched_class) *(__dl_sched_class) *(__rt_sched_class) *(__fair_sched_class) *(__ext_sched_class) *(__idle_sched_class) __sched_class_lowest = .; . = ALIGN(8); __start_ro_after_init = .; *(.data..ro_after_init) . = ALIGN(8); __start___jump_table = .; KEEP(*(__jump_table)) __stop___jump_table = .; . = ALIGN(8); __start_static_call_sites = .; KEEP(*(.static_call_sites)) __stop_static_call_sites = .; __start_static_call_tramp_key = .; KEEP(*(.static_call_tramp_key)) __stop_static_call_tramp_key = .; __end_ro_after_init = .; . = ALIGN(8); __start___tracepoints_ptrs = .; KEEP(*(__tracepoints_ptrs)) __stop___tracepoints_ptrs = .; *(__tracepoints_strings) } .rodata1 : AT(ADDR(.rodata1) - 0xffffffff80000000) { *(.rodata1) } .pci_fixup : AT(ADDR(.pci_fixup) - 0xffffffff80000000) { __start_pci_fixups_early = .; KEEP(*(.pci_fixup_early)) __end_pci_fixups_early = .; __start_pci_fixups_header = .; KEEP(*(.pci_fixup_header)) __end_pci_fixups_header = .; __start_pci_fixups_final = .; KEEP(*(.pci_fixup_final)) __end_pci_fixups_final = .; __start_pci_fixups_enable = .; KEEP(*(.pci_fixup_enable)) __end_pci_fixups_enable = .; __start_pci_fixups_resume = .; KEEP(*(.pci_fixup_resume)) __end_pci_fixups_resume = .; __start_pci_fixups_suspend = .; KEEP(*(.pci_fixup_suspend)) __end_pci_fixups_suspend = .; __start_pci_fixups_resume_early = .; KEEP(*(.pci_fixup_resume_early)) __end_pci_fixups_resume_early = .; __start_pci_fixups_suspend_late = .; KEEP(*(.pci_fixup_suspend_late)) __end_pci_fixups_suspend_late = .; } .builtin_fw : AT(ADDR(.builtin_fw) - 0xffffffff80000000) ALIGN(8) { __start_builtin_fw = .; KEEP(*(.builtin_fw)) __end_builtin_fw = .; } __ksymtab : AT(ADDR(__ksymtab) - 0xffffffff80000000) { __start___ksymtab = .; KEEP(*(SORT(___ksymtab+*))) __stop___ksymtab = .; } __kcrctab : AT(ADDR(__kcrctab) - 0xffffffff80000000) { __start___kcrctab = .; KEEP(*(SORT(___kcrctab+*))) __stop___kcrctab = .; } __kflagstab : AT(ADDR(__kflagstab) - 0xffffffff80000000) { __start___kflagstab = .; KEEP(*(SORT(___kflagstab+*))) __stop___kflagstab = .; } __ksymtab_strings : AT(ADDR(__ksymtab_strings) - 0xffffffff80000000) { *(__ksymtab_strings) } __init_rodata : AT(ADDR(__init_rodata) - 0xffffffff80000000) { *(.ref.rodata) } __param : AT(ADDR(__param) - 0xffffffff80000000) { __start___param = .; KEEP(*(__param)) __stop___param = .; } __modver : AT(ADDR(__modver) - 0xffffffff80000000) { __start___modver = .; KEEP(*(__modver)) __stop___modver = .; } . = ALIGN(16); __ex_table : AT(ADDR(__ex_table) - 0xffffffff80000000) { __start___ex_table = .; KEEP(*(__ex_table)) __stop___ex_table = .; } /DISCARD/ : { *(.note.GNU-stack) *(.note.gnu.property) } .notes : AT(ADDR(.notes) - 0xffffffff80000000) { __start_notes = .; KEEP(*(.note.*)) __stop_notes = .; } :text :note __restore_ph : { *(.__restore_ph) } :text . = ALIGN(((1 << 12))); __end_rodata = .;
 . = ALIGN((1 << 21)); __end_rodata_hpage_align = .; __end_rodata_aligned = .;
 .data : AT(ADDR(.data) - 0xffffffff80000000) {
  _sdata = .;
  . = ALIGN(((1 << 12) << (2 + 1))); __start_init_stack = .; init_thread_union = .; init_stack = .; KEEP(*(.data..init_thread_info)) . = __start_init_stack + ((1 << 12) << (2 + 1)); __end_init_stack = .;
  __top_init_kernel_stack = __end_init_stack - (2 * 8) - 168;
  . = ALIGN((1 << 12)); *(.data..page_aligned) . = ALIGN((1 << 12));
  . = ALIGN((1 << (6))); *(SORT_BY_ALIGNMENT(.data..hot.*)) . = ALIGN((1 << (6)));
  . = ALIGN((1 << (6))); *(.data..cacheline_aligned)
  *(.xiptext) *(.data .data.[0-9a-zA-Z_]* .data.rel.* .data..L* .data..compoundliteral* .data.$__unnamed_* .data.$L*) *(.data..decrypted) *(.ref.data) *(.data..shared_aligned) *(.data..unlikely) __start_once = .; *(.data..once) __end_once = .; *(.data..do_once) . = ALIGN(32); *(__tracepoints) . = ALIGN(8); __start___dyndbg_classes = .; KEEP(*(__dyndbg_classes)) __stop___dyndbg_classes = .; __start___dyndbg = .; KEEP(*(__dyndbg)) __stop___dyndbg = .; __start___trace_bprintk_fmt = .; KEEP(*(__trace_printk_fmt)) __stop___trace_bprintk_fmt = .; . = ALIGN(32); __start__bpf_raw_tp = .; KEEP(*(__bpf_raw_tp_map)) __stop__bpf_raw_tp = .; __start___tracepoint_str = .; KEEP(*(__tracepoint_str)) __stop___tracepoint_str = .; . = ALIGN(8); __kunit_suites_start = .; KEEP(*(.kunit_test_suites)) __kunit_suites_end = .;
  CONSTRUCTORS
  . = ALIGN(0x100); __relocate_kernel_start = .; *(.text..relocate_kernel); *(.data..relocate_kernel); __relocate_kernel_end = .;
  . = ALIGN((1 << 6)); *(.data..read_mostly) . = ALIGN((1 << 6));
  _edata = .;
 } :data
 . = ALIGN(8); __bug_table : AT(ADDR(__bug_table) - 0xffffffff80000000) { __start___bug_table = .; KEEP(*(__bug_table)) __stop___bug_table = .; }
 .orc_header : AT(ADDR(.orc_header) - 0xffffffff80000000) { __start_orc_header = .; KEEP(*(.orc_header)) __stop_orc_header = .; } . = ALIGN(4); .orc_unwind_ip : AT(ADDR(.orc_unwind_ip) - 0xffffffff80000000) { __start_orc_unwind_ip = .; KEEP(*(.orc_unwind_ip)) __stop_orc_unwind_ip = .; } . = ALIGN(2); .orc_unwind : AT(ADDR(.orc_unwind) - 0xffffffff80000000) { __start_orc_unwind = .; KEEP(*(.orc_unwind)) __stop_orc_unwind = .; } text_size = _etext - _stext; . = ALIGN(4); .orc_lookup : AT(ADDR(.orc_lookup) - 0xffffffff80000000) { orc_lookup = .; . += (((text_size + (1 << 8) - 1) / (1 << 8)) + 1) * 4; orc_lookup_end = .; }
 . = ALIGN((1 << 12));
 .init.begin : AT(ADDR(.init.begin) - 0xffffffff80000000) {
  __init_begin = .;
 }
 . = ALIGN((1 << 12)); .init.text : AT(ADDR(.init.text) - 0xffffffff80000000) { _sinittext = .; *(.init.text .init.text.*) *(.text.startup .text.startup.*) _einittext = .; }
 .altinstr_aux : AT(ADDR(.altinstr_aux) - 0xffffffff80000000) {
  *(.altinstr_aux)
  . = ALIGN((1 << 12));
  __inittext_end = .;
 }
 .init.data : AT(ADDR(.init.data) - 0xffffffff80000000) { KEEP(*(SORT(___kentry+*))) *(.init.data .init.data.*) . = ALIGN(8); __ctors_start = .; KEEP(*(SORT(.ctors.*))) KEEP(*(.ctors)) KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array)) __ctors_end = .; . = ALIGN(8); __start_mcount_loc = .; KEEP(*(__mcount_loc)) __stop_mcount_loc = .; ftrace_ops_list_func = arch_ftrace_ops_list_func; *(.init.rodata .init.rodata.*) . = ALIGN(8); __start_ftrace_events = .; KEEP(*(_ftrace_events)) __stop_ftrace_events = .; __start_ftrace_eval_maps = .; KEEP(*(_ftrace_eval_map)) __stop_ftrace_eval_maps = .; . = ALIGN(8); __start_syscalls_metadata = .; KEEP(*(__syscalls_metadata)) __stop_syscalls_metadata = .; . = ALIGN(8); __start_kprobe_blacklist = .; KEEP(*(_kprobe_blacklist)) __stop_kprobe_blacklist = .; . = ALIGN(8); __clk_of_table = .; KEEP(*(__clk_of_table)) KEEP(*(__clk_of_table_end)) . = ALIGN(8); __cpu_method_of_table = .; KEEP(*(__cpu_method_of_table)) KEEP(*(__cpu_method_of_table_end)) . = ALIGN(8); __cpuidle_method_of_table = .; KEEP(*(__cpuidle_method_of_table)) KEEP(*(__cpuidle_method_of_table_end)) . = ALIGN(32); __dtb_start = .; KEEP(*(.dtb.init.rodata)) __dtb_end = .; . = ALIGN(8); __irqchip_acpi_probe_table = .; KEEP(*(__irqchip_acpi_probe_table)) __irqchip_acpi_probe_table_end = .; . = ALIGN(8); __timer_acpi_probe_table = .; KEEP(*(__timer_acpi_probe_table)) __timer_acpi_probe_table_end = .; . = ALIGN(8); __governor_thermal_table = .; KEEP(*(__governor_thermal_table)) __governor_thermal_table_end = .; . = ALIGN(8); __earlycon_table = .; KEEP(*(__earlycon_table)) __earlycon_table_end = .; . = ALIGN(8); __start_lsm_info = .; KEEP(*(.lsm_info.init)) __end_lsm_info = .; . = ALIGN(8); __start_early_lsm_info = .; KEEP(*(.early_lsm_info.init)) __end_early_lsm_info = .; . = ALIGN(8); __kunit_init_suites_start = .; KEEP(*(.kunit_init_test_suites)) __kunit_init_suites_end = .; . = ALIGN(16); __setup_start = .; KEEP(*(.init.setup)) __setup_end = .; __initcall_start = .; KEEP(*(.initcallearly.init)) __initcall0_start = .; KEEP(*(.initcall0.init)) KEEP(*(.initcall0s.init)) __initcall1_start = .; KEEP(*(.initcall1.init)) KEEP(*(.initcall1s.init)) __initcall2_start = .; KEEP(*(.initcall2.init)) KEEP(*(.initcall2s.init)) __initcall3_start = .; KEEP(*(.initcall3.init)) KEEP(*(.initcall3s.init)) __initcall4_start = .; KEEP(*(.initcall4.init)) KEEP(*(.initcall4s.init)) __initcall5_start = .; KEEP(*(.initcall5.init)) KEEP(*(.initcall5s.init)) __initcallrootfs_start = .; KEEP(*(.initcallrootfs.init)) KEEP(*(.initcallrootfss.init)) __initcall6_start = .; KEEP(*(.initcall6.init)) KEEP(*(.initcall6s.init)) __initcall7_start = .; KEEP(*(.initcall7.init)) KEEP(*(.initcall7s.init)) __initcall_end = .; __con_initcall_start = .; KEEP(*(.con_initcall.init)) __con_initcall_end = .; . = ALIGN(4); __initramfs_start = .; KEEP(*(.init.ramfs)) . = ALIGN(8); KEEP(*(.init.ramfs.info)) }
 .x86_cpu_dev.init : AT(ADDR(.x86_cpu_dev.init) - 0xffffffff80000000) {
  __x86_cpu_dev_start = .;
  *(.x86_cpu_dev.init)
  __x86_cpu_dev_end = .;
 }
 . = ALIGN(8);
 .retpoline_sites : AT(ADDR(.retpoline_sites) - 0xffffffff80000000) {
  __retpoline_sites = .;
  *(.retpoline_sites)
  __retpoline_sites_end = .;
 }
 . = ALIGN(8);
 .return_sites : AT(ADDR(.return_sites) - 0xffffffff80000000) {
  __return_sites = .;
  *(.return_sites)
  __return_sites_end = .;
 }
 . = ALIGN(8);
 .call_sites : AT(ADDR(.call_sites) - 0xffffffff80000000) {
  __call_sites = .;
  *(.call_sites)
  __call_sites_end = .;
 }
 . = ALIGN(8);
 .ibt_endbr_seal : AT(ADDR(.ibt_endbr_seal) - 0xffffffff80000000) {
  __ibt_endbr_seal = .;
  *(.ibt_endbr_seal)
  __ibt_endbr_seal_end = .;
 }
 . = ALIGN(8);
 .altinstructions : AT(ADDR(.altinstructions) - 0xffffffff80000000) {
  __alt_instructions = .;
  *(.altinstructions)
  __alt_instructions_end = .;
 }
 .altinstr_replacement : AT(ADDR(.altinstr_replacement) - 0xffffffff80000000) {
  *(.altinstr_replacement)
 }
 . = ALIGN(8);
 .apicdrivers : AT(ADDR(.apicdrivers) - 0xffffffff80000000) {
  __apicdrivers = .;
  *(.apicdrivers);
  __apicdrivers_end = .;
 }
 . = ALIGN(8);
 .exit.text : AT(ADDR(.exit.text) - 0xffffffff80000000) {
  *(.exit.text) *(.text.exit .text.exit.*)
 }
 .exit.data : AT(ADDR(.exit.data) - 0xffffffff80000000) {
  *(.exit.data .exit.data.*) *(.fini_array .fini_array.*) *(.dtors .dtors.*)
 }
 . = ALIGN((1 << 12)); .data..percpu : AT(ADDR(.data..percpu) - 0xffffffff80000000) { __per_cpu_start = .; . = ALIGN((1 << 12)); *(.data..percpu..page_aligned) . = ALIGN((1 << (6))); __per_cpu_hot_start = .; *(SORT_BY_ALIGNMENT(.data..percpu..hot.*)) __per_cpu_hot_end = .; . = ALIGN((1 << (6))); *(.data..percpu..read_mostly) . = ALIGN((1 << (6))); *(.data..percpu) *(.data..percpu..shared_aligned) . = ALIGN((1 << 12)); *(.data..percpu..decrypted) . = ALIGN((1 << 12)); __per_cpu_end = .; }
 ASSERT(__per_cpu_hot_end - __per_cpu_hot_start <= 64, "percpu cache hot data too large")
 . = ALIGN(8); runtime_shift_d_hash_shift : AT(ADDR(runtime_shift_d_hash_shift) - 0xffffffff80000000) { __start_runtime_shift_d_hash_shift = .; KEEP(*(runtime_shift_d_hash_shift)) __stop_runtime_shift_d_hash_shift = .; } . = ALIGN(8); runtime_ptr_dentry_hashtable : AT(ADDR(runtime_ptr_dentry_hashtable) - 0xffffffff80000000) { __start_runtime_ptr_dentry_hashtable = .; KEEP(*(runtime_ptr_dentry_hashtable)) __stop_runtime_ptr_dentry_hashtable = .; } . = ALIGN(8); runtime_ptr___dentry_cache : AT(ADDR(runtime_ptr___dentry_cache) - 0xffffffff80000000) { __start_runtime_ptr___dentry_cache = .; KEEP(*(runtime_ptr___dentry_cache)) __stop_runtime_ptr___dentry_cache = .; } . = ALIGN(8); runtime_ptr___names_cache : AT(ADDR(runtime_ptr___names_cache) - 0xffffffff80000000) { __start_runtime_ptr___names_cache = .; KEEP(*(runtime_ptr___names_cache)) __stop_runtime_ptr___names_cache = .; } . = ALIGN(8); runtime_ptr___filp_cache : AT(ADDR(runtime_ptr___filp_cache) - 0xffffffff80000000) { __start_runtime_ptr___filp_cache = .; KEEP(*(runtime_ptr___filp_cache)) __stop_runtime_ptr___filp_cache = .; } . = ALIGN(8); runtime_ptr___bfilp_cache : AT(ADDR(runtime_ptr___bfilp_cache) - 0xffffffff80000000) { __start_runtime_ptr___bfilp_cache = .; KEEP(*(runtime_ptr___bfilp_cache)) __stop_runtime_ptr___bfilp_cache = .; } . = ALIGN(8); runtime_shift___futex_shift : AT(ADDR(runtime_shift___futex_shift) - 0xffffffff80000000) { __start_runtime_shift___futex_shift = .; KEEP(*(runtime_shift___futex_shift)) __stop_runtime_shift___futex_shift = .; } . = ALIGN(8); runtime_mask___futex_mask : AT(ADDR(runtime_mask___futex_mask) - 0xffffffff80000000) { __start_runtime_mask___futex_mask = .; KEEP(*(runtime_mask___futex_mask)) __stop_runtime_mask___futex_mask = .; } . = ALIGN(8); runtime_ptr___futex_queues : AT(ADDR(runtime_ptr___futex_queues) - 0xffffffff80000000) { __start_runtime_ptr___futex_queues = .; KEEP(*(runtime_ptr___futex_queues)) __stop_runtime_ptr___futex_queues = .; }
 . = ALIGN(8); runtime_ptr_USER_PTR_MAX : AT(ADDR(runtime_ptr_USER_PTR_MAX) - 0xffffffff80000000) { __start_runtime_ptr_USER_PTR_MAX = .; KEEP(*(runtime_ptr_USER_PTR_MAX)) __stop_runtime_ptr_USER_PTR_MAX = .; }
 . = ALIGN((1 << 12));
 .init.end : AT(ADDR(.init.end) - 0xffffffff80000000) {
  __init_end = .;
 }
 .data_nosave : AT(ADDR(.data_nosave) - 0xffffffff80000000) {
  . = ALIGN((1 << 12)); __nosave_begin = .; *(.data..nosave) . = ALIGN((1 << 12)); __nosave_end = .;
 }
 . = ALIGN((1 << 12));
 .bss : AT(ADDR(.bss) - 0xffffffff80000000) {
  __bss_start = .;
  *(.bss..page_aligned)
  . = ALIGN((1 << 12));
  *(.bss .bss.[0-9a-zA-Z_]* .bss..L* .bss..compoundliteral*)
  . = ALIGN((1 << 21)); __start_bss_decrypted = .; __pi___start_bss_decrypted = .; *(.bss..decrypted); . = ALIGN((1 << 12)); __start_bss_decrypted_unused = .; . = ALIGN((1 << 21)); __end_bss_decrypted = .; __pi___end_bss_decrypted = .;
  . = ALIGN((1 << 12));
  __bss_stop = .;
 }
 __end_of_kernel_reserve = .;
 . = ALIGN((1 << 12));
 .brk : AT(ADDR(.brk) - 0xffffffff80000000) {
  __brk_base = .;
  . += 64 * 1024;
  *(.bss..brk)
  __brk_limit = .;
 }
 . = ALIGN((1 << 12));
 _end = .;
 __pi__end = .;
 . = ALIGN((1 << 21));
 .init.scratch : AT(ADDR(.init.scratch) - 0xffffffff80000000) {
  __init_scratch_begin = .;
  *(.init.scratch)
  . = ALIGN((1 << 21));
  __init_scratch_end = .;
 }
 .stab 0 : { *(.stab) } .stabstr 0 : { *(.stabstr) } .stab.excl 0 : { *(.stab.excl) } .stab.exclstr 0 : { *(.stab.exclstr) } .stab.index 0 : { *(.stab.index) } .stab.indexstr 0 : { *(.stab.indexstr) }
 .debug 0 : { *(.debug) } .line 0 : { *(.line) } .debug_srcinfo 0 : { *(.debug_srcinfo) } .debug_sfnames 0 : { *(.debug_sfnames) } .debug_aranges 0 : { *(.debug_aranges) } .debug_pubnames 0 : { *(.debug_pubnames) } .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) } .debug_abbrev 0 : { *(.debug_abbrev) } .debug_line 0 : { *(.debug_line) } .debug_frame 0 : { *(.debug_frame) } .debug_str 0 : { *(.debug_str) } .debug_loc 0 : { *(.debug_loc) } .debug_macinfo 0 : { *(.debug_macinfo) } .debug_pubtypes 0 : { *(.debug_pubtypes) } .debug_ranges 0 : { *(.debug_ranges) } .debug_weaknames 0 : { *(.debug_weaknames) } .debug_funcnames 0 : { *(.debug_funcnames) } .debug_typenames 0 : { *(.debug_typenames) } .debug_varnames 0 : { *(.debug_varnames) } .debug_gnu_pubnames 0 : { *(.debug_gnu_pubnames) } .debug_gnu_pubtypes 0 : { *(.debug_gnu_pubtypes) } .debug_types 0 : { *(.debug_types) } .debug_addr 0 : { *(.debug_addr) } .debug_line_str 0 : { *(.debug_line_str) } .debug_loclists 0 : { *(.debug_loclists) } .debug_macro 0 : { *(.debug_macro) } .debug_names 0 : { *(.debug_names) } .debug_rnglists 0 : { *(.debug_rnglists) } .debug_str_offsets 0 : { *(.debug_str_offsets) }
 .modinfo : { *(.modinfo) . = ALIGN(8); }
 .comment 0 : { *(.comment) } .symtab 0 : { *(.symtab) } .strtab 0 : { *(.strtab) } .shstrtab 0 : { *(.shstrtab) } .klp.symid 0 : { *(.klp.symid) }
 /DISCARD/ : { *(.exitcall.exit) *(.eh_frame) *(__patchable_function_entries) *(.discard) *(.discard.*) *(.export_symbol) *(.no_trim_symbol) *(.gnu.version*) *(__tracepoint_check) }
 .got.plt (INFO) : { *(.got.plt) }
 ASSERT(SIZEOF(.got.plt) == 0 ||
        SIZEOF(.got.plt) == 0x18,
        "Unexpected GOT/PLT entries detected!")
 .got : {
  *(.got) *(.igot.*)
 }
 ASSERT(SIZEOF(.got) == 0, "Unexpected GOT entries detected!")
 .plt : {
  *(.plt) *(.plt.*) *(.iplt)
 }
 ASSERT(SIZEOF(.plt) == 0, "Unexpected run-time procedure linkages detected!")
 .rel.dyn : {
  *(.rel.*) *(.rel_*)
 }
 ASSERT(SIZEOF(.rel.dyn) == 0, "Unexpected run-time relocations (.rel) detected!")
 .rela.dyn : {
  *(.rela.*) *(.rela_*)
 }
 ASSERT(SIZEOF(.rela.dyn) == 0, "Unexpected run-time relocations (.rela) detected!")
}
. = ASSERT((_end - 0xffffffff80000000 <= (1024 * 1024 * 1024)),
    "kernel image bigger than KERNEL_IMAGE_SIZE");
PROVIDE(__ref_stack_chk_guard = __stack_chk_guard);
. = ASSERT((retbleed_return_thunk & 0x3f) == 0, "retbleed_return_thunk not cacheline-aligned");
. = ASSERT((srso_safe_ret & 0x3f) == 0, "srso_safe_ret not cacheline-aligned");
. = ASSERT(((ABSOLUTE(srso_alias_untrain_ret) | srso_alias_safe_ret) -
  (ABSOLUTE(srso_alias_untrain_ret) & srso_alias_safe_ret)) == ((1 << 2) | (1 << 8) | (1 << 14) | (1 << 20)),
  "SRSO function pair won't alias");
. = ASSERT(__x86_indirect_its_thunk_rax & 0x20, "__x86_indirect_thunk_rax not in second half of cacheline");
. = ASSERT(((__x86_indirect_its_thunk_rcx - __x86_indirect_its_thunk_rax) % 64) == 0, "Indirect thunks are not cacheline apart");
. = ASSERT(__x86_indirect_its_thunk_array == __x86_indirect_its_thunk_rax, "Gap in ITS thunk array");
. = ASSERT(its_return_thunk & 0x20, "its_return_thunk not in second half of cacheline");
xen_elfnote_entry_value =
 ABSOLUTE(xen_elfnote_entry) + ABSOLUTE(startup_xen);
xen_elfnote_phys32_entry_value =
 ABSOLUTE(xen_elfnote_phys32_entry) + ABSOLUTE(pvh_start_xen - 0xffffffff80000000);
PROVIDE(early_set_pages_state = __pi_early_set_pages_state);
PROVIDE(early_snp_set_memory_private = __pi_early_snp_set_memory_private);
PROVIDE(early_snp_set_memory_shared = __pi_early_snp_set_memory_shared);
PROVIDE(get_hv_features = __pi_get_hv_features);
PROVIDE(sev_es_terminate = __pi_sev_es_terminate);
PROVIDE(snp_cpuid = __pi_snp_cpuid);
PROVIDE(snp_cpuid_get_table = __pi_snp_cpuid_get_table);
PROVIDE(svsm_issue_call = __pi_svsm_issue_call);
PROVIDE(svsm_process_result_codes = __pi_svsm_process_result_codes);

