#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
TARGET="${1:-pi4}"
IP="${IP:-192.168.1.199}"
RUSER="${RUSER:-pi}"
KERNEL_SRC="${KERNEL_SRC:-$SCRIPT_DIR/raspberry-kernel-sources}"
KERNEL_GIT_URL="${KERNEL_GIT_URL:-https://github.com/raspberrypi/linux.git}"
KERNEL_GIT_BRANCH="${KERNEL_GIT_BRANCH:-rpi-6.18.y}"
STCP_ROOT="${STCP_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
STCP_SRC="${STCP_SRC:-${STCP_LINUX_MODULE_ROOT:-${LINUX_MODULE_ROOT:-$STCP_ROOT/linux-kernel/linux-module}}}"
OUT_DIR="${OUT_DIR:-$SCRIPT_DIR/packages}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
ARCH=arm64
JOBS="${JOBS:-$(nproc)}"
CLEAN="${CLEAN:-0}"

TS=$(date +"%Y%m%d-%H%M%S")
GIT=$(git rev-parse --short HEAD 2>/dev/null || echo nogit)

LOCALVERSION="${LOCALVERSION:--stcp-rpi}"
LOCALVERSION="${LOCALVERSION}-$GIT"

RUST_TOOLCHAIN="${RUST_TOOLCHAIN:-nightly}"

case "$TARGET" in
  pi4) DEFCONFIG="${DEFCONFIG:-bcm2711_defconfig}" ;;
  pi5) DEFCONFIG="${DEFCONFIG:-bcm2712_defconfig}" ;;
  *) echo "Usage: $0 pi4|pi5" >&2; exit 2 ;;
esac

die(){ echo "[FAIL] $*" >&2; exit 1; }
find_cmd(){ command -v "$1" 2>/dev/null || { [[ -x /usr/sbin/$1 ]] && echo /usr/sbin/$1; }; }
need_cmd(){ find_cmd "$1" >/dev/null || die "Missing command: $1"; }

check_kernel() {
	
	(
		cd "$1"

		# Pakotetaan generointi ÄLÄ poista näitä!
		rm -f include/config/auto.conf.cmd
		rm -f include/generated/autoconf.h
		rm -f include/config/kernel.release
		rm -f include/generated/utsrelease.h


		LOCALVERSION="$LOCALVERSION" make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" olddefconfig
	)
}

ensure_crypto_config() {
    echo "[INFO] Ensuring STCP crypto kernel options..."
    (
       cd "$1"

       scripts/config --enable CONFIG_CRYPTO
       scripts/config --enable CONFIG_CRYPTO_SUPPORT

       LOCALVERSION="$LOCALVERSION" make ARCH="$ARCH" \
           CROSS_COMPILE="$CROSS_COMPILE" \
           olddefconfig
    )
}

check_crypto_config() {
    local ok=1

	# NOP now

    (( ok )) || exit 1
}

for c in make tar git rustup strings "${CROSS_COMPILE}gcc" "${CROSS_COMPILE}ld" "${CROSS_COMPILE}ar" "${CROSS_COMPILE}readelf"; do need_cmd "$c"; done

ensure_kernel_source() {
    if [[ -f "$KERNEL_SRC/Makefile" ]]; then
        return 0
    fi

    if [[ -e "$KERNEL_SRC" ]]; then
        die "Kernel source path exists but is not a valid kernel tree: $KERNEL_SRC"
    fi

    echo "[INFO] Raspberry kernel source tree missing."
    echo "[INFO] Cloning: $KERNEL_GIT_URL"
    echo "[INFO] Branch : $KERNEL_GIT_BRANCH"
    echo "[INFO] Target : $KERNEL_SRC"

    mkdir -p "$(dirname "$KERNEL_SRC")"

    git clone         --branch "$KERNEL_GIT_BRANCH"         --single-branch         "$KERNEL_GIT_URL"         "$KERNEL_SRC"

    [[ -f "$KERNEL_SRC/Makefile" ]] ||
        die "Clone completed but kernel Makefile is missing: $KERNEL_SRC"

    [[ -f "$KERNEL_SRC/drivers/nvme/target/Kconfig" ]] ||
        die "Clone is incomplete: drivers/nvme/target/Kconfig is missing"

    echo "[INFO] Raspberry kernel clone ready."
    git -C "$KERNEL_SRC" log -1 --oneline
}

ensure_kernel_source

[[ -f "$STCP_SRC/Makefile" ]] || die "STCP source tree not found: $STCP_SRC"
[[ -d "$STCP_SRC/src" ]] || die "STCP C source tree missing: $STCP_SRC/src"
[[ -d "$STCP_SRC/rust/src" ]] || die "STCP Rust source tree missing: $STCP_SRC/rust/src"

OLD_RPI_SRC="$SCRIPT_DIR/raspberry-kernel-module"
if [[ -d "$OLD_RPI_SRC/src" || -d "$OLD_RPI_SRC/rust/src" ]]; then
    die "Legacy Raspberry STCP source fork still exists at $OLD_RPI_SRC. Raspberry must build from $STCP_SRC"
fi

echo "[INFO] Raspberry kernel source : $KERNEL_SRC"
echo "[INFO] Unified STCP source     : $STCP_SRC"
echo "[INFO] Target                  : $TARGET ($ARCH)"
echo "[INFO] Cross compiler          : $CROSS_COMPILE"
echo "[INFO] LOCALVERSION            : $LOCALVERSION"

rustup toolchain list | grep -q "^${RUST_TOOLCHAIN}" || die "Install nightly: rustup toolchain install $RUST_TOOLCHAIN --component rust-src"


mkdir -p "$OUT_DIR"
[[ -w "$OUT_DIR" ]] || die "Output directory is not writable: $OUT_DIR"

check_kernel "$KERNEL_SRC"

cd "$KERNEL_SRC"
if [[ "$CLEAN" == 1 ]]; then make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" mrproper; fi
if [[ ! -f .config ]]; then 

	cp ../rpi-working.config .config

	ensure_crypto_config "$KERNEL_SRC";

	LOCALVERSION="$LOCALVERSION" yes "" | make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" "$DEFCONFIG";
fi

check_crypto_config;

#if [[ -x scripts/config ]]; then
#  scripts/config --set-str LOCALVERSION "$LOCALVERSION"
#  scripts/config --disable LOCALVERSION_AUTO
#fi

make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" olddefconfig
grep -q '^CONFIG_ARM64=y' .config || die "Kernel config is not ARM64"
grep -q '^CONFIG_MODULES=y' .config || die "Kernel modules are disabled"
LOCALVERSION="$LOCALVERSION" pncwrap -t "STCPv2/Raspberry Pi build"  make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" -j"$JOBS" Image modules
KREL="$(LOCALVERSION="$LOCALVERSION" make -s ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" kernelrelease)"
[[ -f Module.symvers ]] || die "Module.symvers missing"
[[ -f arch/arm64/boot/Image ]] || die "Kernel Image missing"

echo "== Rebuilding STCP against $KREL =="
LOCALVERSION="$LOCALVERSION" pncwrap -t "STCPv2/Raspberry Pi STCP clean build" make -C "$KERNEL_SRC" M="$STCP_SRC" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" clean || true

rm -rf "$STCP_SRC/rust/target"
rm -f "$STCP_SRC/src/rust_core.o" "$STCP_SRC/src/.rust_core.o.cmd" "$STCP_SRC/stcp.ko" "$STCP_SRC/stcp.o"

LOCALVERSION="$LOCALVERSION" \
  pncwrap -t "STCPv2/Raspberry Pi STCP build" \
     make -C "$STCP_SRC" \
        LOCALVERSION="$LOCALVERSION" \
        PLATFORM=rpi \
        KDIR="$KERNEL_SRC" \
        ARCH="$ARCH" \
        CROSS_COMPILE="$CROSS_COMPILE" \
        RUST_TOOLCHAIN="$RUST_TOOLCHAIN" \
        -j"$JOBS" module

STCP_KO="$STCP_SRC/stcp.ko"
[[ -f "$STCP_KO" ]] || die "stcp.ko missing"
MACHINE="$("${CROSS_COMPILE}readelf" -h "$STCP_KO" | awk -F: '/Machine:/{gsub(/^[ \t]+/,"",$2);print $2}')"
[[ "$MACHINE" == AArch64 ]] || die "Wrong architecture: $MACHINE"
VERMAGIC="$(strings "$STCP_KO" | sed -n 's/^vermagic=//p' | head -n1)"
case "$VERMAGIC" in "$KREL "*|"$KREL") ;; *) die "vermagic mismatch: $VERMAGIC" ;; esac

WORK="$(mktemp -d)"; trap 'rm -rf "$WORK"' EXIT
PKG_NAME="stcp-rpi-${TARGET}-${KREL}"
PKG_DIR="$WORK/$PKG_NAME"
ROOTFS="$PKG_DIR/rootfs"
mkdir -p "$ROOTFS/lib/modules/$KREL/extra" "$ROOTFS/boot/firmware"
LOCALVERSION="$LOCALVERSION" make -C "$KERNEL_SRC" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" INSTALL_MOD_PATH="$ROOTFS" modules_install
install -m0644 "$STCP_KO" "$ROOTFS/lib/modules/$KREL/extra/stcp.ko"
install -m0644 "$KERNEL_SRC/arch/arm64/boot/Image" "$ROOTFS/boot/firmware/kernel-stcp-${KREL}.img"
DEPMOD="$(find_cmd depmod || true)"
if [[ -n "$DEPMOD" ]]; then "$DEPMOD" -b "$ROOTFS" -a "$KREL" || sudo "$DEPMOD" -b "$ROOTFS" -a "$KREL"; fi
printf '%s\n' "$KREL" > "$PKG_DIR/KERNEL_RELEASE"
printf '%s\n' "$TARGET" > "$PKG_DIR/TARGET"
cp "$KERNEL_SRC/.config" "$PKG_DIR/kernel.config"
cp "$KERNEL_SRC/Module.symvers" "$PKG_DIR/Module.symvers"
cat > "$PKG_DIR/manifest.txt" <<MANIFEST
package=$PKG_NAME
target=$TARGET
kernel_release=$KREL
stcp_vermagic=$VERMAGIC
dtbs_installed=no
overlays_installed=no
MANIFEST

cat > "$PKG_DIR/install.sh" <<'INSTALL'
#!/usr/bin/env bash
set -Eeuo pipefail
[[ "$(id -u)" == 0 ]] || { echo "Run: sudo ./install.sh" >&2; exit 1; }
PKG_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOTFS="$PKG_DIR/rootfs"
KREL="$(cat "$PKG_DIR/KERNEL_RELEASE")"
KERNEL_FILE="kernel-stcp-${KREL}.img"
if [[ -d /boot/firmware ]]; then BOOT_DIR=/boot/firmware; else BOOT_DIR=/boot; fi
CONFIG="$BOOT_DIR/config.txt"
[[ -f "$CONFIG" ]] || { echo "Missing $CONFIG" >&2; exit 1; }
STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP="/root/stcp-kernel-backup-$STAMP"
mkdir -p "$BACKUP"
cp -a "$CONFIG" "$BACKUP/config.txt"
rm -rf "/lib/modules/$KREL"
mkdir -p "/lib/modules/$KREL"
cp -a "$ROOTFS/lib/modules/$KREL/." "/lib/modules/$KREL/"
install -m0644 "$ROOTFS/boot/firmware/$KERNEL_FILE" "$BOOT_DIR/$KERNEL_FILE"
sed -i '/^# BEGIN STCP KERNEL$/,/^# END STCP KERNEL$/d' "$CONFIG"
cat >> "$CONFIG" <<CFG

# BEGIN STCP KERNEL
[all]
arm_64bit=1
kernel=$KERNEL_FILE
# END STCP KERNEL
CFG
DEPMOD="$(command -v depmod 2>/dev/null || true)"; [[ -n "$DEPMOD" ]] || DEPMOD=/usr/sbin/depmod
"$DEPMOD" -a "$KREL"
echo "Installed $KREL. DTBs and overlays were not modified."
echo "Reboot: sudo reboot"
echo "Rollback: restore $BACKUP/config.txt"
INSTALL
chmod +x "$PKG_DIR/install.sh"
PACKAGE="$OUT_DIR/${PKG_NAME}.tar.gz"
rm -f "$PACKAGE"
tar -C "$WORK" -czf "$PACKAGE" "$PKG_NAME"
echo "Package ready: $PACKAGE"

# Install and reboot Raspberry exactly once.
REMOTE_PACKAGE="${PKG_NAME}.tar.gz"

echo "[INFO] Copying package to ${RUSER}@${IP}:~/${REMOTE_PACKAGE}"
scp "$PACKAGE" "${RUSER}@${IP}:~/${REMOTE_PACKAGE}"

echo "[INFO] Installing package on Raspberry..."
ssh "${RUSER}@${IP}" bash -s -- "$REMOTE_PACKAGE" "$PKG_NAME" <<'REMOTE'
set -Eeuo pipefail

REMOTE_PACKAGE="$1"
PKG_NAME="$2"

rm -rf "$PKG_NAME"
tar xzf "$REMOTE_PACKAGE"

# Remove only previous STCP kernel images; install.sh installs the new one.
sudo rm -f /boot/firmware/kernel-stcp-*.img 2>/dev/null || true
sudo rm -f /boot/kernel-stcp-*.img 2>/dev/null || true

cd "$PKG_NAME"
sudo bash install.sh
cd ..

rm -rf "$PKG_NAME" "$REMOTE_PACKAGE"

echo "[INFO] Package installed; rebooting Raspberry..."
sudo reboot
REMOTE

pncnote -a "STCPv2/Raspberry Pi" \
    -t "Raspberry Pi @ $IP" \
    "Raspberry rebooting!" \
    "$(printf 'Raspberry kernel + unified STCP module update completed\nRebooting....')"

echo "[INFO] Package installed to Raspberry; reboot in progress."
