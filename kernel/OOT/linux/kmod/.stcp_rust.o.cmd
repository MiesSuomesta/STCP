savedcmd_stcp_rust.o := ld -m elf_x86_64 -z noexecstack --no-warn-rwx-segments   -r -o stcp_rust.o @stcp_rust.mod 
