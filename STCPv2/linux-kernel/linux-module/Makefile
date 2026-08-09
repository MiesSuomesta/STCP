# STCPv2 unified Linux kernel module build
#
# One C/Rust source tree is used for both x86_64 Linux and Raspberry Pi ARM64.
# Select only the build environment/toolchain:
#   make PLATFORM=host
#   make PLATFORM=rpi KDIR=/path/to/rpi/kernel CROSS_COMPILE=aarch64-linux-gnu-
#
# On an ARM64 Raspberry Pi itself:
#   make PLATFORM=rpi KDIR=/lib/modules/$(uname -r)/build CROSS_COMPILE=

PLATFORM ?= host
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(CURDIR)
V ?= 1
JOBS ?= $(shell nproc)

RUST_DIR       := $(PWD)/rust
RUSTUP         ?= rustup
RUST_TOOLCHAIN ?= nightly
RUST_OBJ_DIR   := $(PWD)/.rust-objects
RUST_CORE_OBJ  := $(PWD)/src/rust_core.o

ifeq ($(PLATFORM),host)
ARCH ?= x86_64
LLVM ?= 1
RUST_TARGET ?= x86_64-unknown-none
RUSTFLAGS_KERNEL := \
	-C relocation-model=static \
	-C code-model=kernel \
	-C panic=abort \
	-C force-frame-pointers=yes \
	-C no-redzone=yes \
	-Z plt=yes
AR_TOOL ?= llvm-ar
LD_TOOL ?= ld.lld
READELF_TOOL ?= readelf
KBUILD_TOOL_ARGS := LLVM=$(LLVM)
else ifeq ($(PLATFORM),rpi)
ARCH := arm64
CROSS_COMPILE ?= aarch64-linux-gnu-
RUST_TARGET ?= aarch64-unknown-none
RUSTFLAGS_KERNEL := \
	-C relocation-model=static \
	-C code-model=small \
	-C panic=abort \
	-C force-frame-pointers=yes \
	-C target-feature=-neon
AR_TOOL ?= $(if $(CROSS_COMPILE),$(CROSS_COMPILE)ar,ar)
LD_TOOL ?= $(if $(CROSS_COMPILE),$(CROSS_COMPILE)ld,ld)
READELF_TOOL ?= $(if $(CROSS_COMPILE),$(CROSS_COMPILE)readelf,readelf)
KBUILD_TOOL_ARGS := ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
else
$(error Unknown PLATFORM='$(PLATFORM)'; use PLATFORM=host or PLATFORM=rpi)
endif

RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/release
RUST_STATICLIB := $(RUST_OUT_DIR)/libstcp_kernel_core.a

# Probe the *target kernel's socket API*, not the build host and not the whole
# include tree.  Older kernels can contain the text "sockaddr_unsized" in
# unrelated/generated headers even though struct proto_ops still uses
# struct sockaddr *.  linux/net.h is the authoritative API consumed by this
# module, so use it as the feature probe.
STCP_HAVE_SOCKADDR_UNSIZED := $(shell \
	grep -Eq 'sockaddr_unsized' "$(KDIR)/include/linux/net.h" 2>/dev/null \
	&& echo 1 || echo 0)
KBUILD_STCP_ARGS := STCP_HAVE_SOCKADDR_UNSIZED=$(STCP_HAVE_SOCKADDR_UNSIZED)

KERNEL_RELEASE := $(shell $(MAKE) -sC "$(KDIR)" kernelrelease 2>/dev/null || uname -r)
SUDO ?= sudo

.PHONY: all module rust rust-core check-tools check-relocations clean distclean zip \
        host rpi show-config

all: module
host:
	$(MAKE) PLATFORM=host module
rpi:
	$(MAKE) PLATFORM=rpi module

show-config:
	@echo "PLATFORM=$(PLATFORM)"
	@echo "KDIR=$(KDIR)"
	@echo "ARCH=$(ARCH)"
	@echo "CROSS_COMPILE=$(CROSS_COMPILE)"
	@echo "RUST_TARGET=$(RUST_TARGET)"
	@echo "KERNEL_RELEASE=$(KERNEL_RELEASE)"
	@echo "STCP_HAVE_SOCKADDR_UNSIZED=$(STCP_HAVE_SOCKADDR_UNSIZED)"

zip: clean test-clean
	rm -f paketti.zip
	zip -r paketti.zip . -x 'rust/target/*' '.rust-objects/*' '*.o' '*.ko' '*.cmd'

module: rust-core
	$(MAKE) -C "$(KDIR)" M="$(PWD)" $(KBUILD_TOOL_ARGS) $(KBUILD_STCP_ARGS) V="$(V)" -j"$(JOBS)" modules

rust: check-tools
	cd "$(RUST_DIR)" && \
	RUSTFLAGS="$(RUSTFLAGS_KERNEL)" \
	"$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo build \
		-Zbuild-std=core,alloc \
		--target "$(RUST_TARGET)" \
		--release

rust-core: rust
	@test -f "$(RUST_STATICLIB)" || { echo "Missing Rust static library: $(RUST_STATICLIB)"; exit 1; }
	rm -rf "$(RUST_OBJ_DIR)"
	mkdir -p "$(RUST_OBJ_DIR)"
	cd "$(RUST_OBJ_DIR)" && "$(AR_TOOL)" x "$(RUST_STATICLIB)"
	@set -e; \
	objs="$$(find "$(RUST_OBJ_DIR)" -type f -name '*.o' -print)"; \
	test -n "$$objs" || { echo "No Rust objects in static library"; exit 1; }; \
	"$(LD_TOOL)" -r $$objs -o "$(RUST_CORE_OBJ)"
	@printf '%s\n' 'cmd_src/rust_core.o := prelinked Rust static library' > "$(PWD)/src/.rust_core.o.cmd"
	@echo "Rust core ready: $(RUST_CORE_OBJ)"

check-relocations: rust-core
ifeq ($(PLATFORM),host)
	@if "$(READELF_TOOL)" -rW "$(RUST_CORE_OBJ)" | grep -q R_X86_64_GOTPCREL; then \
		"$(READELF_TOOL)" -rW "$(RUST_CORE_OBJ)" | grep R_X86_64_GOTPCREL; exit 1; \
	fi
	@echo "No unsupported x86_64 GOTPCREL relocations"
else
	@"$(READELF_TOOL)" -h "$(RUST_CORE_OBJ)" | grep -q 'Machine:.*AArch64' || { \
		echo "Rust core is not AArch64"; exit 1; }
	@echo "AArch64 Rust core relocation/architecture check passed"
endif

check-tools:
	@command -v "$(RUSTUP)" >/dev/null || { echo "rustup missing"; exit 1; }
	@"$(RUSTUP)" toolchain list | grep -q '^$(RUST_TOOLCHAIN)' || { echo "Rust nightly missing"; exit 1; }
	@command -v "$(AR_TOOL)" >/dev/null || { echo "$(AR_TOOL) missing"; exit 1; }
	@command -v "$(LD_TOOL)" >/dev/null || { echo "$(LD_TOOL) missing"; exit 1; }
	@test -f "$(KDIR)/Makefile" || { echo "Kernel build directory missing/invalid: $(KDIR)"; exit 1; }

clean:
	@if [ -f "$(KDIR)/Makefile" ]; then \
		$(MAKE) -C "$(KDIR)" M="$(PWD)" $(KBUILD_TOOL_ARGS) clean; \
	fi
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUST_TOOLCHAIN)" cargo clean
	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)" "$(PWD)/src/.rust_core.o.cmd"

distclean: clean
	rm -rf "$(RUST_DIR)/target"

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
SUDO                  ?= sudo

.PHONY: test test-udp test-reliability test-build test-clean

test: module test-build
	@set -e; \
	cleanup() { SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh" 2>/dev/null || true; }; \
	trap cleanup EXIT INT TERM; \
	if grep -q '^stcp ' /proc/modules; then SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh"; fi; \
	$(SUDO) modprobe "$(PWD)/stcp.ko" drop_first_data=1; \
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
	rm -f "$(TEST_BASIC_BIN)" "$(TEST_LARGE_BIN)" "$(TEST_UDP_BIN)" "$(TEST_RELIABILITY_BIN)"

#
# Moduuli funkkarit
#
.PHONY: module-stop module-shutdown module-remove module-install module-reload module-install-support

module-install-support:
	echo "Installing support modules..."
	$(SUDO) modprobe libcurve25519       || true
	$(SUDO) modprobe libchacha20poly1305 || true

module-stop module-shutdown module-remove:
	@SUDO="$(SUDO)" MODULE=stcp "$(PWD)/scripts/graceful-unload.sh"

module-install: module module-install-support
	echo "Installing module..."
	@if [ "$(PLATFORM)" != "host" ] && [ "$(shell uname -m)" != "aarch64" ]; then \
		echo "Refusing local install of cross-built Raspberry module"; exit 1; \
	fi
	$(SUDO) install -D -m 0644 stcp.ko /lib/modules/$(KERNEL_RELEASE)/extra/stcp.ko
	$(SUDO) depmod -a $(KERNEL_RELEASE)
	$(SUDO) modprobe stcp

module-reload: module-remove module-install
	echo "Reload done"

#
# Testaus
#

.PHONY: \
	test-basic \
	test-large \
	test-udp \
	test-loss10 \
	test-loss30 \
	test-delay \
	test-duplicate \
	test-reorder \
	test-all

test-basic:
	@./testing/run-basic.sh

test-large:
	@./testing/run-large.sh

test-udp:
	@./testing/run-udp.sh

test-loss10:
	@./testing/run-reliability.sh --loss 10

test-loss30:
	@./testing/run-reliability.sh --loss 30

test-delay:
	@./testing/run-reliability.sh --delay 250

test-duplicate:
	@./testing/run-reliability.sh --duplicate

test-reorder:
	@./testing/run-reliability.sh --reorder

test-all: \
	test-basic \
	test-large \
	test-udp \
	test-loss10 \
	test-loss30 \
	test-delay \
	test-duplicate \
	test-reorder
	@echo
	@echo "============================================="
	@echo "        ALL STCP TESTS PASSED"
	@echo "============================================="

TEST_STRESS_SRC := $(PWD)/stress/stcp_stress.c
TEST_STRESS_BIN := $(PWD)/stress/stcp-stress

.PHONY: \
	stress-build \
	stress-tcp \
	stress-udp \
	stress-all

stress-build: $(TEST_STRESS_BIN)

$(TEST_STRESS_BIN): $(TEST_STRESS_SRC)
	$(TEST_CC) \
		-O2 \
		-g \
		-Wall \
		-Wextra \
		-Werror \
		-pthread \
		"$<" \
		-o "$@"

stress-tcp: module stress-build
	@set -e; \
	cleanup() { \
		SUDO="$(SUDO)" "$(PWD)/scripts/graceful-unload.sh" 2>/dev/null || true; \
	}; \
	trap cleanup EXIT INT TERM; \
	$(SUDO) modprobe libcurve25519; \
	$(SUDO) modprobe libchacha20poly1305; \
	$(SUDO) insmod "$(PWD)/stcp.ko" || true; \
	bash "$(PWD)/stress/run-stress-tcp.sh"

stress-udp: module stress-build
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
