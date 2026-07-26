#!/usr/bin/env bash
set -Euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
TARGET="${1:-pi4}"
KERNEL_SRC="${KERNEL_SRC:-$SCRIPT_DIR/raspberry-kernel-sources}"
STCP_SRC="${STCP_SRC:-$SCRIPT_DIR/raspberry-kernel-module}"
OUT_DIR="${OUT_DIR:-$SCRIPT_DIR/packages}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
ARCH="arm64"
JOBS="${JOBS:-$(nproc)}"
CLEAN="${CLEAN:-0}"
LOCALVERSION="${LOCALVERSION:--stcp-uusi}"
RUST_TOOLCHAIN="${RUST_TOOLCHAIN:-nightly}"

case "$TARGET" in
    pi4)
        DEFCONFIG="${DEFCONFIG:-bcm2711_defconfig}"
        TARGET_DTB="bcm2711-rpi-4-b.dtb"
        ;;
    pi5)
        DEFCONFIG="${DEFCONFIG:-bcm2712_defconfig}"
        TARGET_DTB="bcm2712-rpi-5-b.dtb"
        ;;
    *)
        echo "Usage: $0 pi4|pi5" >&2
        exit 2
        ;;
esac

die() { echo "[FAIL] $*" >&2; exit 1; }
log() { echo "[INFO] $*"; }
find_cmd() {
    command -v "$1" 2>/dev/null || {
        [[ -x "/usr/sbin/$1" ]] && echo "/usr/sbin/$1"
    }
}
need_cmd() { find_cmd "$1" >/dev/null || die "Missing command: $1"; }

for cmd in make tar git rustup strings find awk sed install gzip \
    "${CROSS_COMPILE}gcc" "${CROSS_COMPILE}ld" \
    "${CROSS_COMPILE}ar" "${CROSS_COMPILE}readelf"; do
    need_cmd "$cmd"
done

[[ -f "$KERNEL_SRC/Makefile" ]] || die "Kernel source tree not found: $KERNEL_SRC"
[[ -f "$STCP_SRC/Makefile" ]] || die "STCP source tree not found: $STCP_SRC"
rustup toolchain list | grep -qE "^${RUST_TOOLCHAIN}([[:space:]-]|$)" || \
    die "Install Rust toolchain: rustup toolchain install '$RUST_TOOLCHAIN' --component rust-src"

mkdir -p "$OUT_DIR"
[[ -w "$OUT_DIR" ]] || die "Output directory is not writable: $OUT_DIR"

cd "$KERNEL_SRC"

if [[ "$CLEAN" == "1" ]]; then
    log "Cleaning kernel tree"
    make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" mrproper
fi

if [[ ! -f .config ]]; then
    log "Creating kernel configuration: $DEFCONFIG"
    make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" "$DEFCONFIG"
fi

if [[ -x scripts/config ]]; then
    scripts/config --set-str LOCALVERSION "$LOCALVERSION"
    scripts/config --disable LOCALVERSION_AUTO
    scripts/config --enable BLK_DEV_INITRD

    # Existing .config files override the selected defconfig. Ensure the
    # platform needed by the requested Raspberry Pi target remains enabled.
    case "$TARGET" in
        pi4)
            scripts/config --enable ARCH_BCM2835
            ;;
        pi5)
            scripts/config --enable ARCH_BCM2835
            ;;
    esac
fi

make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" olddefconfig

grep -q '^CONFIG_ARM64=y' .config || die "Kernel config is not ARM64"
grep -q '^CONFIG_MODULES=y' .config || die "Kernel modules are disabled"
grep -q '^CONFIG_BLK_DEV_INITRD=y' .config || die "Kernel initramfs support is disabled"

log "Building kernel, modules and Device Trees"
make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" -j"$JOBS" Image modules dtbs

# Some Raspberry Pi kernel/Kbuild combinations do not include every board DTB
# in the generic dtbs target. Build the selected board DTB explicitly when
# necessary. Kbuild accepts the path relative to arch/arm64/boot/dts.
TARGET_DTB_REL="broadcom/$TARGET_DTB"
TARGET_DTB_EXPECTED="$KERNEL_SRC/arch/arm64/boot/dts/$TARGET_DTB_REL"
if [[ ! -f "$TARGET_DTB_EXPECTED" ]]; then
    log "Generic dtbs target did not produce $TARGET_DTB; building it explicitly"
    make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" -j"$JOBS" "$TARGET_DTB_REL" || \
        make ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" -j"$JOBS" \
            "arch/arm64/boot/dts/$TARGET_DTB_REL"
fi

KREL="$(make -s ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" kernelrelease)"
KIMAGE="$KERNEL_SRC/arch/arm64/boot/Image"

[[ -n "$KREL" ]] || die "Could not determine kernel release"
[[ -f "$KIMAGE" ]] || die "Kernel Image missing: $KIMAGE"
[[ -f "$KERNEL_SRC/System.map" ]] || die "System.map missing"
[[ -f "$KERNEL_SRC/Module.symvers" ]] || die "Module.symvers missing"
[[ -f "$KERNEL_SRC/.config" ]] || die "Kernel .config missing"

# Raspberry Pi kernel trees have used both arm64 and shared arm DTS layouts.
# Search all boot/dts trees instead of assuming a single fixed path.
mapfile -t DTB_ROOTS < <(find "$KERNEL_SRC/arch" -type d -path '*/boot/dts' -print | sort -u)
((${#DTB_ROOTS[@]} > 0)) || die "No arch/*/boot/dts directories found"

TARGET_DTB_PATH=""
for root in "${DTB_ROOTS[@]}"; do
    candidate="$(find "$root" -type f -name "$TARGET_DTB" -print -quit)"
    if [[ -n "$candidate" ]]; then
        TARGET_DTB_PATH="$candidate"
        break
    fi
done

if [[ -z "$TARGET_DTB_PATH" ]]; then
    TARGET_DTS="${TARGET_DTB%.dtb}.dts"
    SOURCE_DTS="$(find "$KERNEL_SRC/arch" -type f -path '*/boot/dts/*' -name "$TARGET_DTS" -print -quit)"
    if [[ -n "$SOURCE_DTS" ]]; then
        echo "[INFO] Relevant configuration:" >&2
        grep -E '^(CONFIG_ARCH_BCM2835|CONFIG_ARM64|CONFIG_OF|CONFIG_DTC)=' \
            "$KERNEL_SRC/.config" >&2 || true
        die "Source DTS exists but DTB was not built: $SOURCE_DTS. Explicit DTB build also failed; inspect the make error immediately above."
    fi
    echo "[INFO] Available Raspberry Pi DTBs:" >&2
    find "$KERNEL_SRC/arch" -type f -path '*/boot/dts/*' \
        -name 'bcm27*-rpi-*.dtb' -printf '  %p\n' 2>/dev/null | head -n 40 >&2 || true
    die "Target DTB missing from kernel tree/build: $TARGET_DTB"
fi

DTB_SRC="$(dirname "$TARGET_DTB_PATH")"
while [[ "$(basename "$DTB_SRC")" != "dts" && "$DTB_SRC" != "/" ]]; do
    DTB_SRC="$(dirname "$DTB_SRC")"
done
[[ "$(basename "$DTB_SRC")" == "dts" ]] || die "Could not determine DTS root for $TARGET_DTB_PATH"

OVERLAY_SRC=""
for root in "${DTB_ROOTS[@]}"; do
    if [[ -d "$root/overlays" ]] && find "$root/overlays" -maxdepth 1 -type f -name '*.dtbo' -print -quit | grep -q .; then
        OVERLAY_SRC="$root/overlays"
        break
    fi
done

log "Using DTB root: $DTB_SRC"
if [[ -n "$OVERLAY_SRC" ]]; then
    log "Using built overlay root: $OVERLAY_SRC"
else
    log "No built overlay set found; installer will copy the target Pi's existing firmware overlays"
fi
log "Target DTB: $TARGET_DTB_PATH"

log "Rebuilding STCP against kernel $KREL"
make -C "$KERNEL_SRC" M="$STCP_SRC" ARCH="$ARCH" \
    CROSS_COMPILE="$CROSS_COMPILE" clean || true
rm -rf "$STCP_SRC/rust/target"
rm -f "$STCP_SRC/src/rust_core.o" "$STCP_SRC/src/.rust_core.o.cmd" \
    "$STCP_SRC/stcp.ko" "$STCP_SRC/stcp.o"

make -C "$STCP_SRC" KDIR="$KERNEL_SRC" ARCH="$ARCH" \
    CROSS_COMPILE="$CROSS_COMPILE" RUST_TOOLCHAIN="$RUST_TOOLCHAIN" \
    -j"$JOBS"

STCP_KO="$STCP_SRC/stcp.ko"
[[ -f "$STCP_KO" ]] || die "stcp.ko missing"

MACHINE="$("${CROSS_COMPILE}readelf" -h "$STCP_KO" | \
    awk -F: '/Machine:/{gsub(/^[ \t]+/, "", $2); print $2}')"
[[ "$MACHINE" == "AArch64" ]] || die "Wrong stcp.ko architecture: $MACHINE"

VERMAGIC="$(strings "$STCP_KO" | sed -n 's/^vermagic=//p' | head -n1)"
case "$VERMAGIC" in
    "$KREL"|"$KREL "*) ;;
    *) die "stcp.ko vermagic mismatch: '$VERMAGIC', expected '$KREL'" ;;
esac

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

PKG_NAME="stcp-rpi-${TARGET}-${KREL}"
PKG_DIR="$WORK/$PKG_NAME"
ROOTFS="$PKG_DIR/rootfs"
BOOT_PAYLOAD="$PKG_DIR/boot-payload"
MODULE_DIR="$ROOTFS/lib/modules/$KREL"

mkdir -p "$MODULE_DIR/extra" "$BOOT_PAYLOAD/dtbs" "$BOOT_PAYLOAD/overlays"

log "Installing modules into package staging tree"
make -C "$KERNEL_SRC" ARCH="$ARCH" CROSS_COMPILE="$CROSS_COMPILE" \
    INSTALL_MOD_PATH="$ROOTFS" DEPMOD=true modules_install
install -m0644 "$STCP_KO" "$MODULE_DIR/extra/stcp.ko"

# Remove staging-only absolute source/build symlinks if a kernel tree created them.
find "$MODULE_DIR" -maxdepth 1 -type l \( -name build -o -name source \) -delete

log "Collecting kernel boot files"
install -m0644 "$KIMAGE" "$BOOT_PAYLOAD/Image"
install -m0644 "$KERNEL_SRC/System.map" "$BOOT_PAYLOAD/System.map"
install -m0644 "$KERNEL_SRC/.config" "$BOOT_PAYLOAD/config"
install -m0644 "$KERNEL_SRC/Module.symvers" "$BOOT_PAYLOAD/Module.symvers"

# Keep the original DTB directory hierarchy. The installer flattens the
# Raspberry Pi board DTBs into the firmware prefix directory as required.
while IFS= read -r -d '' dtb; do
    root=""
    for candidate_root in "${DTB_ROOTS[@]}"; do
        case "$dtb" in
            "$candidate_root"/*) root="$candidate_root"; break ;;
        esac
    done
    [[ -n "$root" ]] || continue
    rel="${dtb#${root}/}"
    mkdir -p "$BOOT_PAYLOAD/dtbs/$(dirname "$rel")"
    install -m0644 "$dtb" "$BOOT_PAYLOAD/dtbs/$rel"
done < <(find "${DTB_ROOTS[@]}" -type f -name '*.dtb' -print0)

if [[ -n "$OVERLAY_SRC" ]]; then
    while IFS= read -r -d '' overlay; do
        install -m0644 "$overlay" "$BOOT_PAYLOAD/overlays/$(basename "$overlay")"
    done < <(find "$OVERLAY_SRC" -maxdepth 1 -type f \
        \( -name '*.dtbo' -o -name '*.dtb' -o -name 'README' \) -print0)
fi

find "$BOOT_PAYLOAD/dtbs" -type f -name "$TARGET_DTB" -print -quit | grep -q . || \
    die "Target DTB was not copied into package"

printf '%s\n' "$KREL" > "$PKG_DIR/KERNEL_RELEASE"
printf '%s\n' "$TARGET" > "$PKG_DIR/TARGET"
printf '%s\n' "$TARGET_DTB" > "$PKG_DIR/TARGET_DTB"

cat > "$PKG_DIR/manifest.txt" <<MANIFEST
package=$PKG_NAME
target=$TARGET
kernel_release=$KREL
target_dtb=$TARGET_DTB
stcp_vermagic=$VERMAGIC
kernel_image=yes
system_map=yes
kernel_config=yes
module_symvers=yes
modules=yes
dtbs=yes
overlays=packaged_if_available_else_copied_from_target
initramfs=generated_on_target
boot_layout=os_prefix
MANIFEST

cat > "$PKG_DIR/install.sh" <<'INSTALL_SCRIPT'
#!/usr/bin/env bash
set -Eeuo pipefail

trap 'rc=$?; echo "[FAIL] install.sh line=$LINENO command=$BASH_COMMAND rc=$rc" >&2' ERR

log() { echo "[INFO] $*"; }
die() { echo "[FAIL] $*" >&2; exit 1; }

[[ "$(id -u)" == "0" ]] || die "Run this installer as root: sudo ./install.sh"

PKG_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOTFS="$PKG_DIR/rootfs"
BOOT_PAYLOAD="$PKG_DIR/boot-payload"
KREL="$(cat "$PKG_DIR/KERNEL_RELEASE")"
TARGET="$(cat "$PKG_DIR/TARGET")"
TARGET_DTB="$(cat "$PKG_DIR/TARGET_DTB")"
PREFIX="stcp-${KREL}"

if [[ -d /boot/firmware && -f /boot/firmware/config.txt ]]; then
    BOOT_DIR="/boot/firmware"
elif [[ -f /boot/config.txt ]]; then
    BOOT_DIR="/boot"
else
    die "Raspberry Pi boot config was not found"
fi

CONFIG_TXT="$BOOT_DIR/config.txt"
CMDLINE_TXT="$BOOT_DIR/cmdline.txt"
PREFIX_DIR="$BOOT_DIR/$PREFIX"
MODULE_SRC="$ROOTFS/lib/modules/$KREL"
MODULE_DST="/lib/modules/$KREL"

[[ -d "$MODULE_SRC" ]] || die "Packaged modules missing: $MODULE_SRC"
[[ -f "$BOOT_PAYLOAD/Image" ]] || die "Packaged kernel Image missing"
[[ -f "$BOOT_PAYLOAD/System.map" ]] || die "Packaged System.map missing"
[[ -f "$BOOT_PAYLOAD/config" ]] || die "Packaged kernel config missing"
[[ -d "$BOOT_PAYLOAD/dtbs" ]] || die "Packaged DTBs missing"
[[ -d "$BOOT_PAYLOAD/overlays" ]] || die "Overlay payload directory missing"
[[ -f "$CMDLINE_TXT" ]] || die "Missing Raspberry Pi cmdline.txt: $CMDLINE_TXT"

for cmd in depmod update-initramfs awk sed find install cp sync; do
    command -v "$cmd" >/dev/null 2>&1 || \
        [[ -x "/usr/sbin/$cmd" ]] || die "Required target command missing: $cmd"
done

DEPMOD="$(command -v depmod 2>/dev/null || echo /usr/sbin/depmod)"
UPDATE_INITRAMFS="$(command -v update-initramfs 2>/dev/null || echo /usr/sbin/update-initramfs)"

STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP="/root/stcp-kernel-backup-${KREL}-${STAMP}"
mkdir -p "$BACKUP"

log "Backing up boot configuration and conflicting files to $BACKUP"
cp -a "$CONFIG_TXT" "$BACKUP/config.txt"
cp -a "$CMDLINE_TXT" "$BACKUP/cmdline.txt"
[[ -d "$PREFIX_DIR" ]] && cp -r --no-preserve=ownership "$PREFIX_DIR" "$BACKUP/"
[[ -d "$MODULE_DST" ]] && cp -a "$MODULE_DST" "$BACKUP/modules-$KREL"
for f in "/boot/vmlinuz-$KREL" "/boot/System.map-$KREL" \
         "/boot/config-$KREL" "/boot/initrd.img-$KREL"; do
    [[ -e "$f" ]] && cp -a "$f" "$BACKUP/"
done

log "Installing kernel modules"
rm -rf "$MODULE_DST"
mkdir -p "$MODULE_DST"
cp -a "$MODULE_SRC/." "$MODULE_DST/"
"$DEPMOD" -a "$KREL"

log "Installing standard /boot kernel metadata"
install -m0644 "$BOOT_PAYLOAD/Image" "/boot/vmlinuz-$KREL"
install -m0644 "$BOOT_PAYLOAD/System.map" "/boot/System.map-$KREL"
install -m0644 "$BOOT_PAYLOAD/config" "/boot/config-$KREL"

log "Generating target-specific initramfs"
rm -f "/boot/initrd.img-$KREL"
"$UPDATE_INITRAMFS" -c -k "$KREL"
[[ -s "/boot/initrd.img-$KREL" ]] || die "initramfs generation failed"

log "Installing isolated Raspberry Pi firmware boot set"
rm -rf "$PREFIX_DIR"
mkdir -p "$PREFIX_DIR/overlays"
install -m0644 "$BOOT_PAYLOAD/Image" "$PREFIX_DIR/kernel8.img"
install -m0644 "/boot/initrd.img-$KREL" "$PREFIX_DIR/initramfs8"
install -m0644 "$CMDLINE_TXT" "$PREFIX_DIR/cmdline.txt"

# Raspberry Pi firmware expects board DTBs at the prefix root. Copy every
# packaged board DTB there, regardless of whether it came from broadcom/.
while IFS= read -r -d '' dtb; do
    install -m0644 "$dtb" "$PREFIX_DIR/$(basename "$dtb")"
done < <(find "$BOOT_PAYLOAD/dtbs" -type f -name '*.dtb' -print0)

# os_prefix also affects dtoverlay lookups. Seed the isolated prefix with
# the overlays already known to boot on this Raspberry Pi, then overwrite them
# with package-built overlays when the source tree supplied any.
if [[ -d "$BOOT_DIR/overlays" ]]; then
    cp -r --no-preserve=ownership,mode,timestamps "$BOOT_DIR/overlays/." "$PREFIX_DIR/overlays/"
fi
if find "$BOOT_PAYLOAD/overlays" -maxdepth 1 -type f -print -quit | grep -q .; then
    cp -r --no-preserve=ownership,mode,timestamps "$BOOT_PAYLOAD/overlays/." "$PREFIX_DIR/overlays/"
fi

[[ -s "$PREFIX_DIR/kernel8.img" ]] || die "Installed kernel is empty"
[[ -s "$PREFIX_DIR/initramfs8" ]] || die "Installed initramfs is empty"
[[ -f "$PREFIX_DIR/$TARGET_DTB" ]] || die "Installed target DTB missing: $TARGET_DTB"
[[ -n "$(find "$PREFIX_DIR/overlays" -maxdepth 1 -type f -name '*.dtbo' -print -quit)" ]] || \
    die "No overlays available: neither package nor $BOOT_DIR/overlays contained .dtbo files"

log "Activating the new boot set in config.txt"
TMP_CONFIG="$(mktemp "${CONFIG_TXT}.stcp.XXXXXX")"
TMP_CLEAN="$(mktemp)"

awk '
    /^# BEGIN STCP KERNEL$/ { skip=1; next }
    /^# END STCP KERNEL$/   { skip=0; next }
    !skip { print }
' "$CONFIG_TXT" > "$TMP_CLEAN"

cat "$TMP_CLEAN" > "$TMP_CONFIG"
cat >> "$TMP_CONFIG" <<CFG

# BEGIN STCP KERNEL
[all]
arm_64bit=1
os_prefix=${PREFIX}/
kernel=kernel8.img
auto_initramfs=1
# END STCP KERNEL
CFG

# config.txt resides on the firmware partition, commonly vfat. Do not try
# to preserve Unix ownership or mode there.
cp --no-preserve=ownership,mode,timestamps "$TMP_CONFIG" "${CONFIG_TXT}.new"
mv -f "${CONFIG_TXT}.new" "$CONFIG_TXT"
rm -f "$TMP_CONFIG" "$TMP_CLEAN"
sync

grep -Fxq "os_prefix=${PREFIX}/" "$CONFIG_TXT" ||     die "config.txt verification failed: os_prefix missing"
grep -Fxq "kernel=kernel8.img" "$CONFIG_TXT" ||     die "config.txt verification failed: kernel setting missing"

log "config.txt activated successfully"
grep -A6 -B1 'BEGIN STCP KERNEL' "$CONFIG_TXT"

ROLLBACK="/usr/local/sbin/rollback-stcp-kernel-${KREL}.sh"
cat > "$ROLLBACK" <<ROLLBACK_SCRIPT
#!/usr/bin/env bash
set -Eeuo pipefail
[[ "\$(id -u)" == "0" ]] || { echo "Run as root" >&2; exit 1; }
cp -a "$BACKUP/config.txt" "$CONFIG_TXT"
rm -rf "$PREFIX_DIR"
if [[ -d "$BACKUP/$(basename "$PREFIX_DIR")" ]]; then
    cp -a "$BACKUP/$(basename "$PREFIX_DIR")" "$PREFIX_DIR"
fi
rm -rf "$MODULE_DST"
if [[ -d "$BACKUP/modules-$KREL" ]]; then
    cp -a "$BACKUP/modules-$KREL" "$MODULE_DST"
fi
"$DEPMOD" -a "$KREL" || true
sync
echo "Restored the previous boot configuration from $BACKUP"
echo "Reboot when ready."
ROLLBACK_SCRIPT
chmod 0755 "$ROLLBACK"

sync

cat <<SUMMARY

Installation completed successfully.

Kernel release:  $KREL
Target:          $TARGET
Boot directory:  $BOOT_DIR
Boot prefix:     $PREFIX_DIR
Modules:         $MODULE_DST
Initramfs:       /boot/initrd.img-$KREL
Backup:          $BACKUP
Rollback:        sudo $ROLLBACK

Before reboot, verify with:
  ls -lh "$PREFIX_DIR/kernel8.img" "$PREFIX_DIR/initramfs8"
  ls -lh "$PREFIX_DIR/$TARGET_DTB"
  grep -A6 -B1 'BEGIN STCP KERNEL' "$CONFIG_TXT"

Then reboot:
  sudo reboot
SUMMARY
INSTALL_SCRIPT
chmod 0755 "$PKG_DIR/install.sh"

cat > "$PKG_DIR/verify-package.sh" <<'VERIFY_SCRIPT'
#!/usr/bin/env bash
set -Euo pipefail
PKG_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
KREL="$(cat "$PKG_DIR/KERNEL_RELEASE")"
TARGET_DTB="$(cat "$PKG_DIR/TARGET_DTB")"
[[ -s "$PKG_DIR/boot-payload/Image" ]]
[[ -s "$PKG_DIR/boot-payload/System.map" ]]
[[ -s "$PKG_DIR/boot-payload/config" ]]
[[ -f "$PKG_DIR/rootfs/lib/modules/$KREL/extra/stcp.ko" ]]
find "$PKG_DIR/boot-payload/dtbs" -type f -name "$TARGET_DTB" -print -quit | grep -q .
echo "Package contents OK for kernel $KREL"
VERIFY_SCRIPT
chmod 0755 "$PKG_DIR/verify-package.sh"

"$PKG_DIR/verify-package.sh"

PACKAGE="$OUT_DIR/${PKG_NAME}.tar.gz"
rm -f "$PACKAGE"
tar -C "$WORK" -czf "$PACKAGE" "$PKG_NAME"

log "Package ready: $PACKAGE"
log "Install on Raspberry Pi with: tar -xzf $(basename "$PACKAGE") && cd $PKG_NAME && sudo ./install.sh"

scp -v "$PACKAGE" pi@192.168.1.199:~/
ssh pi@192.168.1.199 "
    set -o pipefail
    rm -rf '$PKG_NAME'
    tar -xzf '$(basename "$PACKAGE")'
    cd '$PKG_NAME'
    sudo ./install.sh 2>&1 | tee ~/stcp-install-${KREL}.log
"
