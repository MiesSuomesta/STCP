#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
KERNEL_SRC="${KERNEL_SRC:-$SCRIPT_DIR/kernel-source-tree/rust-enabled-linux}"
STCP_SRC="${STCP_SRC:-$SCRIPT_DIR/linux-module}"
OUT_DIR="${OUT_DIR:-$SCRIPT_DIR/packages}"
JOBS="${JOBS:-$(nproc)}"
CLEAN="${CLEAN:-0}"

TS=$(date +"%m%d-%H%M%S")
GIT=$(git rev-parse --short HEAD 2>/dev/null || echo nogit)

LOCALVERSION="${LOCALVERSION:--stcp-kuumin}"
LOCALVERSION="${LOCALVERSION}-$GIT"

RUST_TOOLCHAIN="${RUST_TOOLCHAIN:-nightly}"

die(){ echo "[FAIL] $*" >&2; exit 1; }
find_cmd(){ command -v "$1" 2>/dev/null || { [[ -x /usr/sbin/$1 ]] && echo /usr/sbin/$1; }; }
need_cmd(){ find_cmd "$1" >/dev/null || die "Missing command: $1"; }

for c in make tar git rustup strings "gcc" "ld" "ar" "readelf"; do need_cmd "$c"; done
[[ -f "$KERNEL_SRC/Makefile" ]] || ( 
		pncnote -a "STCPv2/Host compile" "New kernel" "Doing kernel cloning...."
		BD=$(dirname "$KERNEL_SRC")
		GN=$(basename "$KERNEL_SRC")
		mkdir -p "$BD"
		(
			cd "$BD"
			pncwrap -t "Rust enabled kernel clone process" -- git clone https://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git "$GN"

			cd "$GN" && (
				pncnote -a "STCPv2/Host compile" "New kernel" "Detached GIT from source tree..."
				mv .git .dot.git
				git add .
				git commit -m "Rust enabled kernel cloned, orginal source-commit"
			)

		) || (
			pncnote -a "STCPv2/Host compile" "New kernel" "Doing kernel cloning....FAILED"
		)

		pncnote -a "STCPv2/Host compile" "New kernel" "Kernel cloned..."

	)

[[ -f "$STCP_SRC/Makefile" ]] || die "STCP source tree not found: $STCP_SRC"
rustup toolchain list | grep -q "^${RUST_TOOLCHAIN}" || die "Install nightly: rustup toolchain install $RUST_TOOLCHAIN --component rust-src"


mkdir -p "$OUT_DIR"
[[ -w "$OUT_DIR" ]] || die "Output directory is not writable: $OUT_DIR"

cd "$KERNEL_SRC"
if [[ "$CLEAN" == 1 ]]; then make LLVM=1 mrproper; fi
if [[ ! -f .config ]]; then 
	cp "$SCRIPT_DIR"/x86.config .config
fi
if [[ -x scripts/config ]]; then
  scripts/config --set-str LOCALVERSION "$LOCALVERSION"
  scripts/config --disable LOCALVERSION_AUTO
  pncnote -a "STCPv2/Host compile" "New kernel" "$(echo -ne "Kernel version set:\n${LOCALVERSION}")"
fi

pncnote -a "STCPv2/Host compile" "New kernel" "Doing kernel configuration...."
yes "" | make LLVM=1 olddefconfig || true

pncwrap -t "STCPv2/Host kernel" -- make LLVM=1 -j"$JOBS" bindeb-pkg

sudo -E pncwrap -t "STCPv2/Host modules" -- make LLVM=1 -j"$JOBS" modules_install

KREL="$(make -s LLVM=1 kernelrelease)"
[[ -f Module.symvers ]] || die "Module.symvers missing"

echo "== Rebuilding STCP against $KREL =="
( 
	pncnote -a "STCPv2/Host compile" "New STCP module for $KREL" "Compiling STCP for $KREL"
	cd "$STCP_SRC" && (

		mkdir -p "$OUT_DIR"

		make LLVM=1 -j"$JOBS" \
		    KDIR="$KERNEL_SRC" \
		    clean

		make LLVM=1 -j"$JOBS" \
		    KDIR="$KERNEL_SRC" \
		    module

		if [ $? -eq 0 ]
		then
			pncnote -a "STCPv2/Host compile" "New STCP module for $KREL" "Compiled STCP module..OK"
		else
			pncnote -a "STCPv2/Host compile" "New STCP module for $KREL" "Compiled STCP module..FAILED"
		fi

		sudo make LLVM=1 -j"$JOBS" \
		    KDIR="$KERNEL_SRC" \
		    module-install

		if [ $? -eq 0 ]
		then
			pncnote -a "STCPv2/Host compile" "New STCP module for $KREL" "Installed STCP for $KREL.."
		else
			pncnote -a "STCPv2/Host compile" "New STCP module for $KREL" "STCP Install FAILED for $KREL.."
		fi

		MODULE_VERMAGIC="$(sudo modinfo -F vermagic "$STCP_SRC/stcp.ko" | awk '{print $1}')"
		pncnote -a "STCPv2/Host compile" "STCPv2 Build check" "$(echo -en "STCP vermagic:\nmodule=$MODULE_VERMAGIC\nkernel=$KREL")"

		if [[ "$MODULE_VERMAGIC" != "$KREL" ]]; then
  	 		die "STCP vermagic mismatch: module=$MODULE_VERMAGIC kernel=$KREL"
		fi
	)


) && (
	KERNEL_PARENT="$(dirname "$KERNEL_SRC")"

	rm -f "$OUT_DIR"/*.deb

	find "$KERNEL_PARENT" \
	    -maxdepth 1 \
	    -type f \
	    -name '*.deb' \
	    -exec cp -v {} "$OUT_DIR/" \;

	cd "$OUT_DIR" && (
		pncnote -a "STCPv2/Host compile" "Installing new kernel: $KREL" "Installing $KREL ..."
		sudo dpkg -i *.deb && 
		if [ $? -eq 0 ]
		then
			pncnote -a "STCPv2/Host compile" "New kernel $KREL" "Kernel $KREL ready, reboot."
		else
			pncnote -a "STCPv2/Host compile" "New kernel $KREL" "Kernel $KREL install failed!"
		fi
	)
)

