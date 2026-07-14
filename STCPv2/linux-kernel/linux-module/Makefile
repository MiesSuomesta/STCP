KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

RUST_DIR       := $(PWD)/rust
RUST_TARGET    := x86_64-unknown-none
RUST_PROFILE   := release
RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/$(RUST_PROFILE)
RUST_STATICLIB := $(RUST_OUT_DIR)/libstcp_kernel_core.a
RUST_OBJ_DIR   := $(PWD)/.rust-objects
RUST_CORE_OBJ  := $(PWD)/src/rust_core.o

CARGO      ?= cargo
RUSTUP     ?= rustup
RUSTUP_TOOLCHAIN ?= nightly
LLVM_AR    ?= llvm-ar
LD_LLD     ?= ld.lld

.PHONY: all module rust rust-core check-tools clean distclean

all: module

module: rust-core
	$(MAKE) -C $(KDIR) M=$(PWD) LLVM=$(LLVM) V=$(V) modules

rust: check-tools
	cd $(RUST_DIR) && \
	RUSTFLAGS="-C relocation-model=static -C panic=abort" \
	$(RUSTUP) run "$(RUSTUP_TOOLCHAIN)" cargo build \
		-Zbuild-std=core,alloc \
		--target $(RUST_TARGET) \
		--release

rust-core: rust
	@test -f "$(RUST_STATICLIB)" || { \
		echo "Virhe: Rust-staticlib puuttuu: $(RUST_STATICLIB)"; \
		exit 1; \
	}
	rm -rf "$(RUST_OBJ_DIR)"
	mkdir -p "$(RUST_OBJ_DIR)"
	cd "$(RUST_OBJ_DIR)" && "$(LLVM_AR)" x "$(RUST_STATICLIB)"
	@set -e; \
	objs="$$(find "$(RUST_OBJ_DIR)" -type f -name '*.o' -print)"; \
	if [ -z "$$objs" ]; then \
		echo "Virhe: staticlib ei sisältänyt objektitiedostoja"; \
		exit 1; \
	fi; \
	"$(LD_LLD)" -r $$objs -o "$(RUST_CORE_OBJ)"
	@echo "Rust core valmis: $(RUST_CORE_OBJ)"

check-tools:
	@command -v "$(RUSTUP)" >/dev/null || { \
		echo "Virhe: rustup puuttuu"; \
		exit 1; \
	}
	@"$(RUSTUP)" toolchain list | grep -q '^$(RUST_TOOLCHAIN)' || { \
		echo "Virhe: Rust-toolchain $(RUST_TOOLCHAIN) puuttuu"; \
		echo "Asenna: rustup toolchain install $(RUST_TOOLCHAIN)"; \
		exit 1; \
	}
	@command -v "$(LLVM_AR)" >/dev/null || { \
		echo "Virhe: $(LLVM_AR) puuttuu"; \
		exit 1; \
	}
	@command -v "$(LD_LLD)" >/dev/null || { \
		echo "Virhe: $(LD_LLD) puuttuu"; \
		exit 1; \
	}
	@test -d "$(KDIR)" || { \
		echo "Virhe: kernel build-hakemisto puuttuu: $(KDIR)"; \
		exit 1; \
	}

clean:
	cd $(RUST_DIR) && $(CARGO) clean
	$(MAKE) -C $(KDIR) M=$(PWD) LLVM=$(LLVM) clean
	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)"

distclean: clean
	cd "$(RUST_DIR)" && "$(CARGO)" clean
