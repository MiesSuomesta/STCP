# STCPv2 unified Linux kernel module Makefile
#
# One source tree for:
#   PLATFORM=host  -> native x86_64 Linux, LLVM/Clang Kbuild
#   PLATFORM=rpi   -> Raspberry Pi ARM64, GCC cross Kbuild
#
# Examples:
#   make PLATFORM=host module
#   make PLATFORM=host module-install
#
#   make PLATFORM=rpi \
#       KDIR=/path/to/raspberry-kernel-sources \
#       CROSS_COMPILE=aarch64-linux-gnu- \
#       module

PWD := $(CURDIR)

PLATFORM ?= host
V        ?= 1
JOBS     ?= $(shell nproc)
SUDO     ?= sudo

RUST_DIR       := $(PWD)/rust
RUSTUP         ?= rustup
RUST_TOOLCHAIN ?= nightly
RUST_OBJ_DIR   := $(PWD)/.rust-objects
RUST_CORE_OBJ  := $(PWD)/src/rust_core.o

# ---------------------------------------------------------------------------
# Platform selection
# ---------------------------------------------------------------------------

ifeq ($(PLATFORM),host)

ARCH          ?= x86_64
CROSS_COMPILE ?=
KDIR          ?= /lib/modules/$(shell uname -r)/build

# Native host build intentionally uses LLVM, matching the existing host setup.
LLVM ?= 1
KBUILD_TOOLCHAIN_ARGS := LLVM=$(LLVM)

RUST_TARGET    := x86_64-unknown-none
RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/release
RUST_STATICLIB := $(RUST_OUT_DIR)/libstcp_kernel_core.a

RUST_AR ?= llvm-ar
RUST_LD ?= ld.lld

READELF ?= readelf

RUSTFLAGS_KERNEL := \
	-C relocation-model=static \
	-C code-model=kernel \
	-C panic=abort \
	-C force-frame-pointers=yes \
	-C no-redzone=yes \
	-Z plt=yes

else ifeq ($(PLATFORM),rpi)

ARCH          ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-

# RPi KDIR must normally be supplied by build-rpi-package.sh.
KDIR ?= $(PWD)/../../RaspberryPI/raspberry-kernel-sources

# IMPORTANT: Do not pass LLVM=1 for the Raspberry build.  The Raspberry
# kernel is built with the GNU AArch64 cross toolchain and its generated
# Kbuild flags must be consumed by GCC, not Clang.
KBUILD_TOOLCHAIN_ARGS :=

CC      := $(CROSS_COMPILE)gcc
LD      := $(CROSS_COMPILE)ld
AR      := $(CROSS_COMPILE)ar
NM      := $(CROSS_COMPILE)nm
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump
STRIP   := $(CROSS_COMPILE)strip
READELF := $(CROSS_COMPILE)readelf

RUST_TARGET    := aarch64-unknown-none
RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/release
RUST_STATICLIB := $(RUST_OUT_DIR)/libstcp_kernel_core.a

RUST_AR := $(AR)
RUST_LD := $(LD)

RUSTFLAGS_KERNEL := \
	-C relocation-model=static \
	-C code-model=small \
	-C panic=abort \
	-C force-frame-pointers=yes \
	-C target-feature=-neon

else
$(error Unknown PLATFORM '$(PLATFORM)'; use PLATFORM=host or PLATFORM=rpi)
endif

KERNEL_RELEASE_HOST := $(shell uname -r)

# ---------------------------------------------------------------------------
# Common build targets
# ---------------------------------------------------------------------------

.PHONY: all module rust rust-core check check-tools check-rust check-module \
	check-relocations clean distclean print-config zip

all: module

print-config:
	@echo "PLATFORM      = $(PLATFORM)"
	@echo "KDIR          = $(KDIR)"
	@echo "ARCH          = $(ARCH)"
	@echo "CROSS_COMPILE = $(CROSS_COMPILE)"
	@echo "KBUILD args   = $(KBUILD_TOOLCHAIN_ARGS)"
	@echo "RUST_TARGET   = $(RUST_TARGET)"
	@echo "RUST_AR       = $(RUST_AR)"
	@echo "RUST_LD       = $(RUST_LD)"

module: check rust-core
	$(MAKE) -C "$(KDIR)" \
		M="$(PWD)" \
		ARCH="$(ARCH)" \
		CROSS_COMPILE="$(CROSS_COMPILE)" \
		$(KBUILD_TOOLCHAIN_ARGS) \
		V="$(V)" \
		-j"$(JOBS)" \
		modules
	@$(MAKE) --no-print-directory PLATFORM="$(PLATFORM)" \
		KDIR="$(KDIR)" ARCH="$(ARCH)" CROSS_COMPILE="$(CROSS_COMPILE)" \
		check-module

rust: check-rust
	cd "$(RUST_DIR)" && \
	RUSTFLAGS="$(RUSTFLAGS_KERNEL)" \
	"$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo build \
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
	cd "$(RUST_OBJ_DIR)" && "$(RUST_AR)" x "$(RUST_STATICLIB)"
	@set -e; \
	objs="$$(find "$(RUST_OBJ_DIR)" -type f -name '*.o' -print)"; \
	test -n "$$objs" || { echo "No Rust objects in static library"; exit 1; }; \
	"$(RUST_LD)" -r $$objs -o "$(RUST_CORE_OBJ)"
	@printf '%s\n' \
		'cmd_src/rust_core.o := $(RUST_LD) -r <Rust objects> -o src/rust_core.o' \
		> "$(PWD)/src/.rust_core.o.cmd"
	@echo "Rust core ready: $(RUST_CORE_OBJ)"

check: check-tools check-rust
	@test -d "$(KDIR)" || { echo "Kernel build directory missing: $(KDIR)"; exit 1; }
	@test -f "$(KDIR)/Makefile" || { echo "Invalid kernel build tree: $(KDIR)"; exit 1; }

check-tools:
	@command -v "$(RUSTUP)" >/dev/null || { echo "rustup missing"; exit 1; }
	@command -v "$(RUST_AR)" >/dev/null || { echo "$(RUST_AR) missing"; exit 1; }
	@command -v "$(RUST_LD)" >/dev/null || { echo "$(RUST_LD) missing"; exit 1; }
ifeq ($(PLATFORM),rpi)
	@command -v "$(CC)" >/dev/null || { echo "$(CC) missing"; exit 1; }
	@command -v "$(LD)" >/dev/null || { echo "$(LD) missing"; exit 1; }
	@command -v "$(AR)" >/dev/null || { echo "$(AR) missing"; exit 1; }
	@command -v "$(READELF)" >/dev/null || { echo "$(READELF) missing"; exit 1; }
endif

check-rust:
	@"$(RUSTUP)" toolchain list | grep -q '^$(RUST_TOOLCHAIN)' || { \
		echo "Rust toolchain missing: $(RUST_TOOLCHAIN)"; \
		echo "Install: rustup toolchain install $(RUST_TOOLCHAIN) --component rust-src"; \
		exit 1; \
	}
	@"$(RUSTUP)" component list --toolchain "$(RUST_TOOLCHAIN)" --installed \
		| grep -q '^rust-src' || { \
		echo "rust-src missing for $(RUST_TOOLCHAIN)"; \
		echo "Install: rustup component add rust-src --toolchain $(RUST_TOOLCHAIN)"; \
		exit 1; \
	}

check-module:
	@test -f "$(PWD)/stcp.ko" || { echo "stcp.ko was not created"; exit 1; }
	@file "$(PWD)/stcp.ko"
ifeq ($(PLATFORM),rpi)
	@"$(READELF)" -h "$(PWD)/stcp.ko" | grep -q 'Machine:.*AArch64' || { \
		echo "ERROR: stcp.ko is not AArch64"; \
		exit 1; \
	}
	@echo "ARM64 module ready: $(PWD)/stcp.ko"
else
	@readelf -h "$(PWD)/stcp.ko" | grep -Eq 'Machine:.*(Advanced Micro Devices X86-64|X86-64)' || { \
		echo "ERROR: stcp.ko is not x86_64"; \
		exit 1; \
	}
	@echo "x86_64 module ready: $(PWD)/stcp.ko"
endif

check-relocations: rust-core
ifeq ($(PLATFORM),host)
	@if readelf -rW "$(RUST_CORE_OBJ)" | grep -q R_X86_64_GOTPCREL; then \
		readelf -rW "$(RUST_CORE_OBJ)" | grep R_X86_64_GOTPCREL; \
		exit 1; \
	fi
	@echo "No unsupported x86_64 GOTPCREL relocations"
else
	@echo "check-relocations: x86_64-specific check skipped for PLATFORM=rpi"
endif

clean:
	@if [ -f "$(KDIR)/Makefile" ]; then \
		$(MAKE) -C "$(KDIR)" \
			M="$(PWD)" \
			ARCH="$(ARCH)" \
			CROSS_COMPILE="$(CROSS_COMPILE)" \
			$(KBUILD_TOOLCHAIN_ARGS) \
			clean; \
	fi
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo clean
	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)" "$(PWD)/src/.rust_core.o.cmd"
	rm -f "$(PWD)/stcp.ko" "$(PWD)/stcp.o"

distclean: clean
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo clean

zip: clean test-clean
	rm -f paketti.zip
	zip -r paketti.zip .

# ---------------------------------------------------------------------------
# Host-only test binaries and tests
# ---------------------------------------------------------------------------

TEST_DIR              := $(PWD)/testing
TEST_BASIC_SRC        := $(TEST_DIR)/stcp_test.c
TEST_BASIC_BIN        := $(TEST_DIR)/stcp_test
TEST_LARGE_SRC        := $(TEST_DIR)/stcp_large_test.c
TEST_LARGE_BIN        := $(TEST_DIR)/stcp_large_test
TEST_PROT_SELECT_SRC  := $(TEST_DIR)/stcp_protocol_select.c
TEST_PROT_SELECT_BIN  := $(TEST_DIR)/stcp_protocol_select
TEST_UDP_SRC          := $(TEST_DIR)/stcp_udp_test.c
TEST_UDP_BIN          := $(TEST_DIR)/stcp_udp_test
TEST_RELIABILITY_SRC  := $(TEST_DIR)/stcp_udp_reliability_test.c
TEST_RELIABILITY_BIN  := $(TEST_DIR)/stcp_udp_reliability_test
TEST_CC               ?= gcc
TEST_TIMEOUT          ?= 20

.PHONY: test test-udp test-reliability test-build test-clean

test:
	@test "$(PLATFORM)" = host || { echo "test is host-only"; exit 2; }
	@$(MAKE) --no-print-directory PLATFORM=host module test-build
	@set -e; \
	cleanup() { SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh" 2>/dev/null || true; }; \
	trap cleanup EXIT INT TERM; \
	if grep -q '^stcp ' /proc/modules; then SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh"; fi; \
	$(SUDO) insmod "$(PWD)/stcp.ko" drop_first_data=1; \
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
	echo "All STCP window/retransmission tests passed"

test-build: $(TEST_BASIC_BIN) $(TEST_LARGE_BIN) $(TEST_PROT_SELECT_BIN) $(TEST_UDP_BIN) $(TEST_RELIABILITY_BIN)

$(TEST_RELIABILITY_BIN): $(TEST_RELIABILITY_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

$(TEST_UDP_BIN): $(TEST_UDP_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

$(TEST_PROT_SELECT_BIN): $(TEST_PROT_SELECT_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

$(TEST_BASIC_BIN): $(TEST_BASIC_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

$(TEST_LARGE_BIN): $(TEST_LARGE_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra "$<" -o "$@"

test-clean:
	rm -f "$(TEST_BASIC_BIN)" "$(TEST_LARGE_BIN)" \
	      "$(TEST_PROT_SELECT_BIN)" "$(TEST_UDP_BIN)" \
	      "$(TEST_RELIABILITY_BIN)"

# ---------------------------------------------------------------------------
# Module lifecycle (host only)
# ---------------------------------------------------------------------------

.PHONY: module-stop module-shutdown module-remove module-install \
	module-reload module-install-support

module-install-support:
	@test "$(PLATFORM)" = host || { echo "module-install is host-only"; exit 2; }
	@echo "Installing support modules..."
	$(SUDO) modprobe libcurve25519 || true
	$(SUDO) modprobe libchacha20poly1305 || true

module-stop module-shutdown module-remove:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@SUDO="$(SUDO)" MODULE=stcp "$(PWD)/scripts/graceful-unload.sh"

module-install: module module-install-support
	@test "$(PLATFORM)" = host || { echo "module-install is host-only"; exit 2; }
	@echo "Installing module to /lib/modules/$(KERNEL_RELEASE_HOST)/extra ..."
	$(SUDO) install -D -m 0644 "$(PWD)/stcp.ko" \
		"/lib/modules/$(KERNEL_RELEASE_HOST)/extra/stcp.ko"
	$(SUDO) depmod -a "$(KERNEL_RELEASE_HOST)"
	$(SUDO) modprobe stcp

module-reload: module-remove module-install
	@echo "Reload done"

# ---------------------------------------------------------------------------
# Existing functional tests (host only)
# ---------------------------------------------------------------------------

.PHONY: test-basic test-large test-udp test-loss10 test-loss30 \
	test-delay test-duplicate test-reorder test-all

test-basic:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-basic.sh

test-large:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-large.sh

test-udp:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-udp.sh

test-loss10:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-reliability.sh --loss 10

test-loss30:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-reliability.sh --loss 30

test-delay:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-reliability.sh --delay 250

test-duplicate:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-reliability.sh --duplicate

test-reorder:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@./testing/run-reliability.sh --reorder

test-all: test-basic test-large test-udp test-loss10 test-loss30 \
	test-delay test-duplicate test-reorder
	@echo
	@echo "============================================="
	@echo "        ALL STCP TESTS PASSED"
	@echo "============================================="

# ---------------------------------------------------------------------------
# Stress tests (host only)
# ---------------------------------------------------------------------------

TEST_STRESS_SRC := $(PWD)/stress/stcp_stress.c
TEST_STRESS_BIN := $(PWD)/stress/stcp-stress

.PHONY: stress-build stress-tcp stress-udp stress-all

stress-build: $(TEST_STRESS_BIN)

$(TEST_STRESS_BIN): $(TEST_STRESS_SRC)
	$(TEST_CC) -O2 -g -Wall -Wextra -Werror -pthread "$<" -o "$@"

stress-tcp:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@$(MAKE) --no-print-directory PLATFORM=host module stress-build
	@set -e; \
	cleanup() { \
		SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh" 2>/dev/null || true; \
	}; \
	trap cleanup EXIT INT TERM; \
	$(SUDO) modprobe libcurve25519; \
	$(SUDO) modprobe libchacha20poly1305; \
	$(SUDO) insmod "$(PWD)/stcp.ko" || true; \
	bash "$(PWD)/stress/run-stress-tcp.sh"

stress-udp:
	@test "$(PLATFORM)" = host || { echo "$@ is host-only"; exit 2; }
	@$(MAKE) --no-print-directory PLATFORM=host module stress-build
	@set -e; \
	cleanup() { \
		SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh" 2>/dev/null || true; \
	}; \
	trap cleanup EXIT INT TERM; \
	$(SUDO) modprobe libcurve25519; \
	$(SUDO) modprobe libchacha20poly1305; \
	$(SUDO) insmod "$(PWD)/stcp.ko" || true; \
	bash "$(PWD)/stress/run-stress-udp.sh"

stress-all: stress-tcp stress-udp
	@echo
	@echo "========================================"
	@echo " ALL STCP STRESS TESTS PASSED"
	@echo "========================================"
