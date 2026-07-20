savedcmd_stcp.o := ld.lld -m elf_x86_64 -z noexecstack   -r -o stcp.o @stcp.mod  ; /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug/tools/objtool/objtool --hacks=jump_label --hacks=noinstr --hacks=skylake --ibt --prefix=16 --mcount --mnop --orc --retpoline --rethunk --sls --static-call --uaccess  --link  --module stcp.o

stcp.o: $(wildcard /usr/src/linux-headers-7.2.0-rc1-next-20260703-rust-stcp-debug/tools/objtool/objtool)
