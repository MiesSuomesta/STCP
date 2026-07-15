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

TEST_DIR        := $(PWD)/testing
TEST_BASIC_SRC  := $(TEST_DIR)/stcp_test.c
TEST_BASIC_BIN  := $(TEST_DIR)/stcp_test
TEST_LARGE_SRC  := $(TEST_DIR)/stcp_large_test.c
TEST_LARGE_BIN  := $(TEST_DIR)/stcp_large_test
TEST_CC         ?= cc
TEST_TIMEOUT    ?= 20
SUDO            ?= sudo

.PHONY: test test-build test-clean

test: module test-build
	@set -e; \
	cleanup() { $(SUDO) rmmod stcp 2>/dev/null || true; }; \
	trap cleanup EXIT INT TERM; \
	if grep -q '^stcp ' /proc/modules; then $(SUDO) rmmod stcp; fi; \
	$(SUDO) insmod "$(PWD)/stcp.ko"; \
	echo "=== Basic STCP handshake test ==="; \
	timeout "$(TEST_TIMEOUT)"s "$(TEST_BASIC_BIN)" server & server_pid=$$!; \
	sleep 1; \
	timeout "$(TEST_TIMEOUT)"s "$(TEST_BASIC_BIN)" client; \
	wait $$server_pid; \
	echo "=== Large framed handshake test ==="; \
	timeout "$(TEST_TIMEOUT)"s "$(TEST_LARGE_BIN)" server & server_pid=$$!; \
	sleep 1; \
	timeout "$(TEST_TIMEOUT)"s "$(TEST_LARGE_BIN)" client; \
	wait $$server_pid; \
	echo "All STCP handshake tests passed"

test-build: $(TEST_BASIC_BIN) $(TEST_LARGE_BIN)

$(TEST_BASIC_BIN): $(TEST_BASIC_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

$(TEST_LARGE_BIN): $(TEST_LARGE_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

test-clean:
	rm -f "$(TEST_BASIC_BIN)" "$(TEST_LARGE_BIN)"

#
# Moduuli funkkarit
#
.PHONY: module-remove module-install module-reload module-install-support

module-install-support:
	echo "Installing support modules..."
	$(SUDO) modprobe libcurve25519       || true
	$(SUDO) modprobe libchacha20poly1305 || true

module-remove:
	echo "Removing module..."
	$(SUDO) rmmod "$(PWD)/stcp.ko" || true

module-install: module module-install-support
	echo "Installing module..."
	$(SUDO) insmod "$(PWD)/stcp.ko" || true

module-reload: module-remove module-install
	echo "Reload done"
