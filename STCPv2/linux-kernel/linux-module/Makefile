KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

LLVM ?= 1
V    ?= 1

RUST_DIR       := $(PWD)/rust
RUST_TARGET    := x86_64-unknown-none
RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/release
RUST_STATICLIB := $(RUST_OUT_DIR)/libstcp_kernel_core.a
RUST_OBJ_DIR   := $(PWD)/.rust-objects
RUST_CORE_OBJ  := $(PWD)/src/rust_core.o

RUSTUP           ?= rustup
RUSTUP_TOOLCHAIN ?= nightly
LLVM_AR          ?= llvm-ar
LD_LLD           ?= ld.lld

RUSTFLAGS_KERNEL := \
	-C relocation-model=static \
	-C code-model=kernel \
	-C panic=abort \
	-C force-frame-pointers=yes \
	-C no-redzone=yes \
	-Z plt=yes

.PHONY: all module rust rust-core check-tools clean distclean check-relocations

all: module

remove-module:
	sudo rmmod stcp

install-module: module
	sudo insmod stcp.ko
	sudo dmesg | tail -n 10

reinstall: remove-module install-module
	@echo Jees...

module: rust-core
	$(MAKE) -C "$(KDIR)" M="$(PWD)" LLVM="$(LLVM)" V="$(V)" modules

rust: check-tools
	cd "$(RUST_DIR)" && \
	RUSTFLAGS="$(RUSTFLAGS_KERNEL)" \
	"$(RUSTUP)" run "$(RUSTUP_TOOLCHAIN)" cargo build \
		-Zbuild-std=core,alloc \
		--target "$(RUST_TARGET)" \
		--release

rust-core: rust
	@test -f "$(RUST_STATICLIB)" || { \
		echo "Missing Rust static library: $(RUST_STATICLIB)"; \
		exit 1; \
	}
	rm -rf "$(RUST_OBJ_DIR)"
	mkdir -p "$(RUST_OBJ_DIR)"
	cd "$(RUST_OBJ_DIR)" && "$(LLVM_AR)" x "$(RUST_STATICLIB)"
	@set -e; \
	objs="$$(find "$(RUST_OBJ_DIR)" -type f -name '*.o' -print)"; \
	test -n "$$objs" || { echo "No Rust objects in static library"; exit 1; }; \
	"$(LD_LLD)" -r $$objs -o "$(RUST_CORE_OBJ)"
	@echo "Rust core ready: $(RUST_CORE_OBJ)"

check-relocations: rust-core
	@if readelf -rW "$(RUST_CORE_OBJ)" | grep -q R_X86_64_GOTPCREL; then \
		readelf -rW "$(RUST_CORE_OBJ)" | grep R_X86_64_GOTPCREL; \
		exit 1; \
	fi
	@echo "No unsupported GOTPCREL relocations"

check-tools:
	@command -v "$(RUSTUP)" >/dev/null || { echo "rustup missing"; exit 1; }
	@"$(RUSTUP)" toolchain list | grep -q '^$(RUSTUP_TOOLCHAIN)' || { \
		echo "Rust nightly missing"; exit 1; \
	}
	@command -v "$(LLVM_AR)" >/dev/null || { echo "$(LLVM_AR) missing"; exit 1; }
	@command -v "$(LD_LLD)" >/dev/null || { echo "$(LD_LLD) missing"; exit 1; }
	@test -d "$(KDIR)" || { echo "Kernel build directory missing: $(KDIR)"; exit 1; }

clean:
	$(MAKE) -C "$(KDIR)" M="$(PWD)" LLVM="$(LLVM)" clean
	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)"

distclean: clean
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUSTUP_TOOLCHAIN)" cargo clean
