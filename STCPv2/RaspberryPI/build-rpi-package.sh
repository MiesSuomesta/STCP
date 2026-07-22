#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
TARGET="${1:-pi4}"
KERNEL_SRC="${KERNEL_SRC:-$SCRIPT_DIR/raspberry-kernel-sources}"
STCP_SRC="${STCP_SRC:-$SCRIPT_DIR/raspberry-kernel-module}"
OUT_DIR="${OUT_DIR:-$SCRIPT_DIR/packages}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
ARCH=arm64
JOBS="${JOBS:-$(nproc)}"
CLEAN="${CLEAN:-0}"
LOCALVERSION="${LOCALVERSION:--stcp}"
RUST_TOOLCHAIN="${RUST_TOOLCHAIN:-nightly}"

case "$TARGET" in
  pi4) DEFCONFIG="${DEFCONFIG:-bcm2711_defconfig}" ;;
  pi5) DEFCONFIG="${DEFCONFIG:-bcm2712_defconfig}" ;;
  *) echo "Usage: $0 pi4|pi5" >&2; exit 2 ;;
esac

die(){ echo "[FAIL] $*" >&2; exit 1; }
find_cmd(){ command -v "$1" 2>/dev/null || { [[ -x /usr/sbin/$1 ]] && echo /usr/sbin/$1; }; }
need_cmd(){ find_cmd "$1" >/dev/null || die "Missing command: $1"; }

for c in make tar git rustup strings "${CROSS_COMPILE}gcc" "${CROSS_COMPILE}ld" "${CROSS_COMPILE}ar" "${CROSS_COMPILE}readelf"; do need_cmd "$c"; done
[[ -f "$KERNEL_SRC/Makefile" ]] || die "Kernel source tree not found: $KERNEL_SRC"
[[ -f "$STCP_SRC/Makefile" ]] || die "STCP source tree not found: $STCP_SRC"
rustup toolchain list | grep -q "^${RUST_TOOLCHAIN}" || die "Install nightly: rustup toolchain install $RUST_TOOLCHAIN --component rust-src"
mkdir -p "$OUT_DIR"
[[ -w "$OUT_DIR" ]] || die "Output directory is not writable: $OUT_DIR"

cd "$KERNEL_SRC"
if [[ "$CLEAN" == 1 ]]; then make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" mrproper; fi
if [[ ! -f .config ]]; then make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" "$DEFCONFIG"; fi
if [[ -x scripts/config ]]; then
  scripts/config --set-str LOCALVERSION "$LOCALVERSION"
  scripts/config --disable LOCALVERSION_AUTO
fi
make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" olddefconfig
grep -q '^CONFIG_ARM64=y' .config || die "Kernel config is not ARM64"
grep -q '^CONFIG_MODULES=y' .config || die "Kernel modules are disabled"
make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" -j"$JOBS" Image modules
KREL="$(make -s ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" kernelrelease)"
[[ -f Module.symvers ]] || die "Module.symvers missing"
[[ -f arch/arm64/boot/Image ]] || die "Kernel Image missing"

echo "== Rebuilding STCP against $KREL =="
make -C "$KERNEL_SRC" M="$STCP_SRC" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" clean || true
rm -rf "$STCP_SRC/rust/target"
rm -f "$STCP_SRC/src/rust_core.o" "$STCP_SRC/src/.rust_core.o.cmd" "$STCP_SRC/stcp.ko" "$STCP_SRC/stcp.o"
make -C "$STCP_SRC" KDIR="$KERNEL_SRC" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" RUST_TOOLCHAIN="$RUST_TOOLCHAIN" -j"$JOBS"
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
make -C "$KERNEL_SRC" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" INSTALL_MOD_PATH="$ROOTFS" modules_install
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
