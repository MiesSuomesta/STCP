# STCPv2 Raspberry Pi ARM64 kernel module
# Cross-compile host: x86_64 Linux
# Target:              Raspberry Pi, AArch64

KDIR ?= $(CURDIR)/../raspberry-kernel-sources
PWD  := $(CURDIR)

ARCH          := arm64
CROSS_COMPILE ?= aarch64-linux-gnu-
JOBS          ?= $(shell nproc)
V             ?= 0

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
AR      := $(CROSS_COMPILE)ar
NM      := $(CROSS_COMPILE)nm
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
STRIP   := $(CROSS_COMPILE)strip
READELF := $(CROSS_COMPILE)readelf

RUST_DIR       ?= $(abspath $(PWD)/../common-rust)
RUST_TARGET    := aarch64-unknown-none
RUST_TOOLCHAIN ?= nightly
RUSTUP         ?= rustup
RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/release
RUST_STATICLIB := $(RUST_OUT_DIR)/libstcp_kernel_core.a
RUST_OBJ_DIR   := $(PWD)/.rust-objects
RUST_CORE_OBJ  := $(PWD)/src/rust_core.o

RUSTFLAGS_KERNEL := \
	-C relocation-model=static \
	-C code-model=small \
	-C panic=abort \
	-C force-frame-pointers=yes \
	-C target-feature=-neon

.PHONY: all module rust rust-core check check-rust check-module clean help

all: module

module: check rust-core
	$(MAKE) -C "$(KDIR)" M="$(PWD)" \
		ARCH="$(ARCH)" \
		CROSS_COMPILE="$(CROSS_COMPILE)" \
		CC="$(CC)" LD="$(LD)" AR="$(AR)" NM="$(NM)" \
		OBJCOPY="$(OBJCOPY)" OBJDUMP="$(OBJDUMP)" STRIP="$(STRIP)" \
		V="$(V)" -j"$(JOBS)" modules
	@$(MAKE) --no-print-directory check-module

rust: check-rust
	cd "$(RUST_DIR)" && \
	RUSTFLAGS="$(RUSTFLAGS_KERNEL)" \
	"$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo build \
		-Zbuild-std=core,alloc \
		--target "$(RUST_TARGET)" \
		--release

rust-core: rust
	@test -f "$(RUST_STATICLIB)" || { \
		echo "Missing Rust static library: $(RUST_STATICLIB)"; exit 1; \
	}
	rm -rf "$(RUST_OBJ_DIR)"
	mkdir -p "$(RUST_OBJ_DIR)"
	cd "$(RUST_OBJ_DIR)" && "$(AR)" x "$(RUST_STATICLIB)"
	@objs="$$(find "$(RUST_OBJ_DIR)" -type f -name '*.o' -print)"; \
	test -n "$$objs" || { echo "No Rust objects in static library"; exit 1; }; \
	"$(LD)" -r $$objs -o "$(RUST_CORE_OBJ)"
	@printf '%s\n' \
		'cmd_src/rust_core.o := $(LD) -r <Rust objects> -o src/rust_core.o' \
		> "$(PWD)/src/.rust_core.o.cmd"
	@echo "Rust core ready: $(RUST_CORE_OBJ)"

check: check-rust
	@test -d "$(KDIR)" || { echo "Kernel tree missing: $(KDIR)"; exit 1; }
	@test -f "$(KDIR)/Makefile" || { echo "Invalid kernel tree: $(KDIR)"; exit 1; }
	@command -v "$(CC)" >/dev/null || { echo "Missing $(CC)"; exit 1; }
	@command -v "$(LD)" >/dev/null || { echo "Missing $(LD)"; exit 1; }
	@command -v "$(AR)" >/dev/null || { echo "Missing $(AR)"; exit 1; }

check-rust:
	@command -v "$(RUSTUP)" >/dev/null || { echo "rustup missing"; exit 1; }
	@"$(RUSTUP)" toolchain list | grep -q '^$(RUST_TOOLCHAIN)' || { \
		echo "Rust toolchain missing: $(RUST_TOOLCHAIN)"; \
		echo "Install: rustup toolchain install $(RUST_TOOLCHAIN) --component rust-src"; \
		exit 1; \
	}
	@"$(RUSTUP)" component list --toolchain "$(RUST_TOOLCHAIN)" --installed \
		| grep -q '^rust-src' || { \
		echo "rust-src missing"; \
		echo "Install: rustup component add rust-src --toolchain $(RUST_TOOLCHAIN)"; \
		exit 1; \
	}

check-module:
	@test -f "$(PWD)/stcp.ko" || { echo "stcp.ko was not created"; exit 1; }
	@file "$(PWD)/stcp.ko"
	@"$(READELF)" -h "$(PWD)/stcp.ko" | grep -q 'Machine:.*AArch64' || { \
		echo "ERROR: stcp.ko is not AArch64"; exit 1; \
	}
	@echo "ARM64 module ready: $(PWD)/stcp.ko"

clean:
	@if [ -f "$(KDIR)/Makefile" ]; then \
		$(MAKE) -C "$(KDIR)" M="$(PWD)" \
			ARCH="$(ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" clean; \
	fi
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo clean
	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)" "$(PWD)/src/.rust_core.o.cmd"

help:
	@echo "make KDIR=/path/to/raspberry-kernel-sources"
	@echo "make clean KDIR=/path/to/raspberry-kernel-sources"
