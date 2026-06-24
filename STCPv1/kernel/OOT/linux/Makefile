
#defaulttina verbose
V:=1
TOPDIR := $(abspath $(CURDIR))
include $(TOPDIR)/Makefile.commit
include $(TOPDIR)/Makefile.common
RATELIMIT := 1
.PHONY: kmod kmod-release modules all clean kclean superclean mkdirs make-build-makefile git-commit-when-no-errors make-cmd rust-build-debug kmod-debug modinfo golden test logs

test:
	sudo bash test.sh 4466

golden:
	@bash scripts/golden

modinfo:
	@modinfo kmod/stcp_rust.ko

git-commit: superclean kmod
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

.PHONY: the-rust rust-build-debug target


rust-build: make-build-makefile
RUST_TOOLCHAIN ?= +nightly

RUST_TARGET_JSON := mun.json
RUST_TARGET_DIR  := mun
RUSTFLAGS_CFG := 'build.rustflags=["-C","panic=abort","-C","relocation-model=static","-C","code-model=kernel","-C","embed-bitcode=no","-C","lto=no","-C","target-cpu=x86-64","-C","target-feature=-fma,-fma4,-avx,-avx2"]'

CARGO_OPTS      := -Z build-std=core,alloc,compiler_builtins \
                   -Z build-std-features=compiler-builtins-mem,no-asm

# Orggis

# RUSTFLAGS_CFG    := 'build.rustflags=["-C","panic=abort","-C","relocation-model=static","-C","code-model=kernel","-C","embed-bitcode=no","-C","lto=no"]'

RUST_OPTS        := +nightly

rust-build:
	# 1) stcp-core
	cd rust && \
	  cargo $(RUST_OPTS) $(CARGO_OPTS) \
	    build \
	      --target $(RUST_TARGET_JSON) \
	      --release \
	      --config $(RUSTFLAGS_CFG)

	bash scripts/rust-ar-to-o.sh \
	  "rust/target/$(RUST_TARGET_DIR)/release/libthe_stcp_kernel_module.a" \
	  "kmod/the_stcp_kernel_module.o" \
	  "kmod/.the_stcp_kernel_module.o.cmd"

rust-build-debug:
	# 1) stcp-core
	cd rust && \
	  cargo $(RUST_OPTS) $(CARGO_OPTS) \
	    build \
	      --features stcp_debug
	      --target $(RUST_TARGET_JSON) \
	      --release \
	      --config $(RUSTFLAGS_CFG)

	bash scripts/rust-ar-to-o.sh \
	  "rust/target/$(RUST_TARGET_DIR)/debug/libthe_stcp_kernel_module.a" \
	  "kmod/the_stcp_kernel_module.o" \
	  "kmod/.the_stcp_kernel_module.o.cmd"

rust-clean:
	cd rust && cargo clean
	@echo "Rust clean: DONE"

kmod-release modules: make-build-makefile rust-build

	$(MAKE) -C "$(KDIR)" M="$(KMOD_SRC)" CFLAGS_MODULE="-DUSE_RATE_LIMIT=1 -DDISABLE_DEBUG -DSTCP_RELEASE" KBUILD_SRC="$(KMOD_SRC)" modules

kmod: make-build-makefile rust-build-debug
	$(MAKE) -C "$(KDIR)" M="$(KMOD_SRC)" SKIP_STACK_VALIDATION=1 \
		  CFLAGS_MODULE="-DUSE_RATE_LIMIT=0"  KBUILD_SRC="$(KMOD_SRC)" modules

all: rust-build kmod-release modinfo
debug: rust-build kmod modinfo

kclean:
	-$(MAKE) -C "$(KDIR)" M="$(KMOD_BUILD)" KBUILD_SRC="$(KMOD_SRC)" clean

clean: kclean rust-clean clean-build

superclean: clean
	@find "$(SRC_DIR)" -name '*.lock' -o -name '*.cmd' -o -name '.tmp_versions' -o -name '*.mod*' -o -name '*.o' | xargs -r rm -rf

zip: superclean
	mv ../*zip ../arkisto/ || true
	zip -r "koodit-$(shell date).zip" *

testround: superclean kmod
	bash dev-start.sh || true

logs:
	scp serveri:~/debug.kernel.log .

logs-clean:
	ssh serveri "echo > ~/debug.kernel.log"

analyse: logs
	cat debug.kernel.log | bash scripts/analyse-oops.sh 2>&1 | tee analysed.txt

commit: superclean git-commit
	@echo "Git commit done..."

$(RUST_CRATE_OBJECT_NAME).o: rust/Cargo.toml
	@echo "[cargo] build the_rust....."
	cd "rust" && $(CARGO_BUILD_CMD)
	bash scripts/mk-cmd.sh
	@echo "[OK] The rust object ($(KERNEL_MODULE_NAME)) ok "

