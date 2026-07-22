KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

LLVM ?= 1
V    ?= 1

RUST_DIR       ?= $(abspath $(PWD)/../common-rust)
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

.PHONY: all module rust rust-core check-tools clean distclean check-relocations zip

zip: clean test-clean
	rm -f paketti.zip
	zip -r paketti.zip .

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
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUSTUP_TOOLCHAIN)" cargo clean
	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)"

distclean: clean
	cd "$(RUST_DIR)" && "$(RUSTUP)" run "$(RUSTUP_TOOLCHAIN)" cargo clean

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
	$(SUDO) modprobe libcurve25519; \
	$(SUDO) modprobe libchacha20poly1305; \
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
	$(SUDO) insmod "$(PWD)/stcp.ko" || true

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
