TARGET ?= x86_64-unknown-none
.PHONY: all clean
all:
	RUSTFLAGS="-C relocation-model=static -C panic=abort" \
	cargo +nightly build -Zbuild-std=core,alloc --target $(TARGET) --release
	mkdir -p target
	cp target/$(TARGET)/release/libstcp_kernel_core.a target/stcp-kernel-core.o
	@echo "NOTE: static archive normally must be converted/linked into one relocatable object for Kbuild."
clean:
	cargo clean
