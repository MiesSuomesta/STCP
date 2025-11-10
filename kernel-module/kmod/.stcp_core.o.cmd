cmd_kmod_stcp_core_o := ld.bfd -r -m elf_x86_64 -o kmod/stcp_core.o rust/target/mun/release/libstcp_core.a 
# in_archive := rust/target/mun/release/libstcp_core.a
# out_object := kmod/stcp_core.o
# undef_opts := 
