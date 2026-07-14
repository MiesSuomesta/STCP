KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

LLVM ?= 1
V    ?= 1

RUST_DIR       := $(PWD)/rust
RUST_TARGET    := x86_64-unknown-none
RUST_PROFILE   := release
RUST_OUT_DIR   := $(RUST_DIR)/target/$(RUST_TARGET)/$(RUST_PROFILE)
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

.PHONY: \
	all \
	module \
	rust \
	rust-core \
	check-tools \
	check-relocations \
	clean \
	distclean

all: module

module: rust-core
	$(MAKE) \
		-C "$(KDIR)" \
		M="$(PWD)" \
		LLVM="$(LLVM)" \
		V="$(V)" \
		modules

rust: check-tools
	cd "$(RUST_DIR)" && \
	RUSTFLAGS="$(RUSTFLAGS_KERNEL)" \
	"$(RUSTUP)" run "$(RUSTUP_TOOLCHAIN)" cargo build \
		-Zbuild-std=core,alloc \
		--target "$(RUST_TARGET)" \
		--release

rust-core: rust
	@test -f "$(RUST_STATICLIB)" || { \
		echo "Virhe: Rust-staticlib puuttuu:"; \
		echo "  $(RUST_STATICLIB)"; \
		exit 1; \
	}

	rm -rf "$(RUST_OBJ_DIR)"
	mkdir -p "$(RUST_OBJ_DIR)"

	cd "$(RUST_OBJ_DIR)" && \
	"$(LLVM_AR)" x "$(RUST_STATICLIB)"

	@set -e; \
	objs="$$(find "$(RUST_OBJ_DIR)" -type f -name '*.o' -print)"; \
	if [ -z "$$objs" ]; then \
		echo "Virhe: staticlib ei sisältänyt objektitiedostoja"; \
		exit 1; \
	fi; \
	"$(LD_LLD)" \
		-r \
		$$objs \
		-o "$(RUST_CORE_OBJ)"

	@echo
	@echo "Rust core valmis:"
	@echo "  $(RUST_CORE_OBJ)"
	@echo

check-relocations: rust-core
	@echo "Tarkistetaan GOTPCREL-relokaatiot..."
	@if readelf -rW "$(RUST_CORE_OBJ)" | \
		grep -q 'R_X86_64_GOTPCREL'; then \
		echo; \
		echo "Virhe: rust_core.o sisältää edelleen GOTPCREL-relokaatioita:"; \
		readelf -rW "$(RUST_CORE_OBJ)" | \
			grep 'R_X86_64_GOTPCREL'; \
		exit 1; \
	fi

	@echo "OK: rust_core.o ei sisällä GOTPCREL-relokaatioita"

check-tools:
	@command -v "$(RUSTUP)" >/dev/null || { \
		echo "Virhe: rustup puuttuu"; \
		exit 1; \
	}

	@"$(RUSTUP)" toolchain list | \
		grep -q '^$(RUSTUP_TOOLCHAIN)' || { \
		echo "Virhe: Rust-toolchain puuttuu:"; \
		echo "  $(RUSTUP_TOOLCHAIN)"; \
		echo; \
		echo "Asenna se komennolla:"; \
		echo "  rustup toolchain install $(RUSTUP_TOOLCHAIN)"; \
		echo "  rustup component add rust-src --toolchain $(RUSTUP_TOOLCHAIN)"; \
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

	@command -v readelf >/dev/null || { \
		echo "Virhe: readelf puuttuu"; \
		exit 1; \
	}

	@test -d "$(KDIR)" || { \
		echo "Virhe: kernel build-hakemisto puuttuu:"; \
		echo "  $(KDIR)"; \
		exit 1; \
	}

clean:
	$(MAKE) \
		-C "$(KDIR)" \
		M="$(PWD)" \
		LLVM="$(LLVM)" \
		clean

	rm -rf "$(RUST_OBJ_DIR)"
	rm -f "$(RUST_CORE_OBJ)"

distclean: clean
	cd "$(RUST_DIR)" && \
	"$(RUSTUP)" run "$(RUSTUP_TOOLCHAIN)" cargo clean

