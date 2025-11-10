
#defaulttina verbose
V:=1
TOPDIR := $(abspath $(CURDIR))
include $(TOPDIR)/Makefile.commit
include $(TOPDIR)/Makefile.common

.PHONY: kmod modules all clean kclean superclean mkdirs make-build-makefile git-commit-when-no-errors make-cmd

git-commit-when-no-errors: verify smoke
	@echo "✅ Build OK — committing to git"
	@git add -A
	@git commit -m "✅ STCP kernel module built successfully at $(DATETS)"
	@git tag -m "✅ STCP OK" "build-ok-$(DATETS)"
	@git push --follow-tags || true

# Rust .o pitää olla M=hakemistossa ennen linkitystä
mkdirs:
	@mkdir -p "$(KMOD_SRC)"
	@mkdir -p "$(KMOD_BUILD)"
	@echo "Hakemistot luotu.."

make-build-makefile: mkdirs
	@cp $(TOPDIR)/Makefile.build build/Makefile
	@echo "Build makefile made ..."

clean-build: rust-clean
	@rm -rf build/
	@echo "Build cleaned..."

.PHONY: the-rust


rust-build: make-build-makefile
RUST_TOOLCHAIN ?= +nightly

RUST_TARGET_JSON := ../mun.json
RUST_TARGET_DIR  := mun
RUSTFLAGS_CFG    := 'build.rustflags=["-C","panic=abort","-C","relocation-model=static","-C","code-model=kernel"]'

rust-build:
	# 1) stcp-core
	cd rust/stcp_core && \
	  cargo +nightly \
	    -Z build-std=alloc,core,compiler_builtins \
	    -Z build-std-features=compiler-builtins-mem \
	    build \
	      --target $(RUST_TARGET_JSON) \
	      --release \
	      --config $(RUSTFLAGS_CFG)

	# Tätä ei tarvo, alla oleva crate imasee mukaan .a:han ?
	#bash scripts/rust-ar-to-o.sh \
	#  "rust/target/$(RUST_TARGET_DIR)/release/libstcp_core.a" \
	#  "kmod/stcp_core.o" \
	#  "kmod/.stcp_core.o.cmd"

	# 2) the_rust_implementation
	cd rust/the_rust_implementation && \
	  cargo +nightly \
	    -Z build-std=alloc,core,compiler_builtins \
	    -Z build-std-features=compiler-builtins-mem \
	    build \
	      --target $(RUST_TARGET_JSON) \
	      --release \
	      --config $(RUSTFLAGS_CFG)

	bash scripts/rust-ar-to-o.sh \
	  "rust/target/$(RUST_TARGET_DIR)/release/libthe_rust_implementation.a" \
	  "kmod/the_rust_implementation.o" \
	  "kmod/.the_rust_implementation.o.cmd"

rust-clean:
	cd rust && cargo clean
	@echo "Rust clean: DONE"

kmod modules: make-build-makefile rust-build make-cmd

	$(MAKE) -C "$(KDIR)" M="$(KMOD_SRC)" KBUILD_SRC="$(KMOD_SRC)" modules

all: rust-build kmod git-commit-when-no-errors

kclean:
	-$(MAKE) -C "$(KDIR)" M="$(KMOD_BUILD)" KBUILD_SRC="$(KMOD_SRC)" clean

clean: kclean rust-clean clean-build

superclean: clean
	@find "$(SRC_DIR)" -name '*.lock' -o -name '*.cmd' -o -name '.tmp_versions' -o -name '*.mod*' -o -name '*.o' | xargs -r rm -rf

make-cmd:
	bash $(TOPDIR)/scripts/mk-cmd.sh 

$(RUST_CRATE_OBJECT_NAME).o: rust/Cargo.toml make-cmd
	@echo "[cargo] build the_rust....."
	cd "rust" && $(CARGO_BUILD_CMD)
	bash scripts/mk-cmd.sh
	@echo "[OK] The rust object ($(KERNEL_MODULE_NAME)) ok "

