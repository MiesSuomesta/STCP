
make clean || true

make rust-build
# tämän jälkeen:
readelf -r kmod/the_rust_implementation.o | grep R_X86_64_GOTPCREL || echo "the_rust_implementation.o: ei GOTPCREL"

make -C /lib/modules/$(uname -r)/build M=$PWD/kmod clean
make -C /lib/modules/$(uname -r)/build M=$PWD/kmod modules

readelf -r kmod/stcp_rust.ko | grep R_X86_64_GOTPCREL || echo "stcp_rust.ko: ei GOTPCREL"

#insmod kmod/stcp_rust.ko
#dmesg | tail -n 40
