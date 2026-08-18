#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/artifacts/build"
ZAPP="$ROOT/zephyr/nordic/application"
SUDO="${SUDO:-sudo}"
PNC_ENABLE="${PNC_ENABLE:-1}"
PNC_APP="${PNC_APP:-STCPv2 install}"
PNC_MOBILE="${PNC_MOBILE:-0}"
PNC_WRAPPER="${PNC_WRAPPER:-}"

find_pnc_wrapper() {
    if [[ -n "$PNC_WRAPPER" ]] && command -v "$PNC_WRAPPER" >/dev/null 2>&1; then
        command -v "$PNC_WRAPPER"
        return 0
    fi
    command -v pncwrap 2>/dev/null || command -v pncwrapper 2>/dev/null || return 1
}

pnc_note() {
    [[ "$PNC_ENABLE" == 1 ]] || return 0
    command -v pncnote >/dev/null 2>&1 || return 0
    local title="$1" body="$2"
    local -a args=(-a "$PNC_APP")
    [[ "$PNC_MOBILE" == 1 ]] && args+=(-m)
    pncnote "${args[@]}" "$title" "$body" >/dev/null 2>&1 || true
}

pnc_run() {
    local title="$1"; shift
    local wrapper=""
    if [[ "$PNC_ENABLE" == 1 ]]; then
        wrapper="$(find_pnc_wrapper || true)"
    fi
    if [[ -n "$wrapper" ]]; then
        "$wrapper" -t "$title" -- "$@"
    else
        "$@"
    fi
}

usage() {
    cat <<USAGE
Usage: $(basename "$0") [host|rpi|kernel|zephyr-nrf9151|zephyr-ethernet|all]

Default: host

  host               Install staged host kernel module on this machine
  rpi                Install staged Raspberry Pi kernel + module tree over SSH
  kernel             Backwards-compatible alias for 'host'
  zephyr-nrf9151     Flash the already-built nRF9151/LTE firmware
  zephyr-ethernet    Flash the already-built nRF9151 + W5500 firmware
  all                Install local kernel module and flash ZEPHYR_VARIANT

Environment:
  ZEPHYR_VARIANT=nrf9151|ethernet   Used by 'all' (default: nrf9151)
  NCS_DIR=PATH                      Nordic Connect SDK tree
  SUDO=command                      local sudo command (default: sudo)
  RPI_HOST=host                     Raspberry Pi SSH host (default: raspi)
  RPI_USER=user                     Raspberry Pi SSH user (default: current user)
  RPI_SUDO=command                  remote sudo command (default: sudo)
  RPI_SSH_OPTS="..."               extra options for ssh/scp
  RPI_REBOOT=1                     reboot Raspberry after successful install (default: 0)
  PNC_ENABLE=0                      disable pncwrap/pncnote integration
  PNC_WRAPPER=CMD                   force pncwrap/pncwrapper command
  PNC_MOBILE=1                      also send pncnote notifications to mobile

Run scripts/build-all.sh first. Zephyr variants are intentionally not both
flashed in sequence because the second flash would simply replace the first.
USAGE
}

install_host() {
    local src release vermagic
    release="$(uname -r)"
    src="$OUT/kernel-host/stcp.ko"

    [[ -f "$src" ]] || {
        echo "[FAIL] Missing staged host module: $src" >&2
        echo "       Build it first with: scripts/build-all.sh host" >&2
        return 1
    }

    if command -v modinfo >/dev/null 2>&1; then
        vermagic="$(modinfo -F vermagic "$src" 2>/dev/null | awk '{print $1}')"
        if [[ -n "$vermagic" && "$vermagic" != "$release" ]]; then
            echo "[FAIL] Kernel mismatch: module=$vermagic running=$release" >&2
            return 1
        fi
    fi

    echo "[INFO] Installing host module for kernel $release"
    pnc_note "STCP host install" "Installing STCP module for kernel $release"
    if grep -q '^stcp ' /proc/modules 2>/dev/null; then
        "$SUDO" modprobe -r stcp
    fi
    "$SUDO" modprobe libcurve25519 || true
    "$SUDO" modprobe libchacha20poly1305 || true
    "$SUDO" install -D -m 0644 "$src" "/lib/modules/$release/extra/stcp.ko"
    "$SUDO" depmod -a "$release"
    "$SUDO" modprobe stcp

    local installed="/lib/modules/$release/extra/stcp.ko"
    if [[ "$(sha256sum "$src" | awk '{print $1}')" != "$(sha256sum "$installed" | awk '{print $1}')" ]]; then
        echo "[FAIL] Installed host module checksum differs" >&2
        return 1
    fi
    echo "[OK] STCP host kernel module installed and loaded"
    pnc_note "STCP host install complete" "STCP module installed and loaded for $release"
}

install_rpi() {
    local stage="$OUT/kernel-rpi"
    local host user remote sudo_remote reboot krel local_module vermagic
    local pkg tmp archive remote_archive
    local -a ssh_opts=()

    host="${RPI_HOST:-raspi}"
    user="${RPI_USER:-pi}"
    sudo_remote="${RPI_SUDO:-sudo}"
    reboot="${RPI_REBOOT:-0}"
    remote="${user}@${host}"

    [[ -s "$stage/Image" ]] || {
        echo "[FAIL] Missing staged Raspberry Pi kernel: $stage/Image" >&2
        echo "       Build first with: scripts/build-all.sh rpi" >&2
        return 1
    }
    [[ -s "$stage/KERNEL_RELEASE" ]] || { echo "[FAIL] Missing $stage/KERNEL_RELEASE" >&2; return 1; }
    krel="$(<"$stage/KERNEL_RELEASE")"
    [[ -n "$krel" ]] || { echo "[FAIL] Empty Raspberry Pi kernel release" >&2; return 1; }
    [[ -d "$stage/rootfs/lib/modules/$krel" ]] || {
        echo "[FAIL] Missing staged module tree: $stage/rootfs/lib/modules/$krel" >&2
        return 1
    }
    local_module="$stage/rootfs/lib/modules/$krel/extra/stcp.ko"
    [[ -s "$local_module" ]] || { echo "[FAIL] Missing staged STCP module: $local_module" >&2; return 1; }

    vermagic="$(modinfo -F vermagic "$local_module" 2>/dev/null | awk '{print $1}' || true)"
    if [[ -n "$vermagic" && "$vermagic" != "$krel" ]]; then
        echo "[FAIL] staged STCP vermagic=$vermagic, staged kernel=$krel" >&2
        return 1
    fi

    if [[ -n "${RPI_SSH_OPTS:-}" ]]; then
        read -r -a ssh_opts <<< "$RPI_SSH_OPTS"
    fi
    command -v ssh >/dev/null 2>&1 || { echo "[FAIL] ssh not found" >&2; return 1; }
    command -v scp >/dev/null 2>&1 || { echo "[FAIL] scp not found" >&2; return 1; }
    command -v tar >/dev/null 2>&1 || { echo "[FAIL] tar not found" >&2; return 1; }

    echo "[INFO] Checking Raspberry Pi target: $remote"
    ssh "${ssh_opts[@]}" "$remote" 'set -e; uname -m; uname -r; test -f /boot/firmware/config.txt -o -f /boot/config.txt' >/dev/null || {
        echo "[FAIL] Raspberry Pi connectivity/boot-layout check failed" >&2
        return 1
    }

    tmp="$(mktemp -d)"
    trap 'rm -rf "${tmp:-}"' RETURN
    pkg="stcp-rpi-install-${krel}"
    mkdir -p "$tmp/$pkg"
    cp -a "$stage/Image" "$tmp/$pkg/Image"
    cp -a "$stage/rootfs" "$tmp/$pkg/rootfs"
    cp -a "$stage/KERNEL_RELEASE" "$tmp/$pkg/KERNEL_RELEASE"
    [[ -f "$stage/System.map" ]] && cp -a "$stage/System.map" "$tmp/$pkg/System.map"
    [[ -f "$stage/kernel.config" ]] && cp -a "$stage/kernel.config" "$tmp/$pkg/kernel.config"

    cat > "$tmp/$pkg/install.sh" <<'REMOTE_INSTALL'
#!/usr/bin/env bash
set -Eeuo pipefail
[[ "$(id -u)" == 0 ]] || { echo "[FAIL] run installer as root" >&2; exit 1; }
PKG_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KREL="$(<"$PKG_DIR/KERNEL_RELEASE")"
MODULE_SRC="$PKG_DIR/rootfs/lib/modules/$KREL"
MODULE_DST="/lib/modules/$KREL"

if [[ -d /boot/firmware && -f /boot/firmware/config.txt ]]; then
    BOOT_DIR=/boot/firmware
elif [[ -f /boot/config.txt ]]; then
    BOOT_DIR=/boot
else
    echo "[FAIL] Raspberry Pi config.txt not found" >&2
    exit 1
fi
CONFIG_TXT="$BOOT_DIR/config.txt"
KERNEL_FILE="kernel-stcp-${KREL}.img"
KERNEL_DST="$BOOT_DIR/$KERNEL_FILE"
DEPMOD="$(command -v depmod 2>/dev/null || true)"
[[ -n "$DEPMOD" ]] || DEPMOD=/usr/sbin/depmod

[[ -s "$PKG_DIR/Image" ]] || { echo "[FAIL] packaged Image missing" >&2; exit 1; }
[[ -d "$MODULE_SRC" ]] || { echo "[FAIL] packaged modules missing" >&2; exit 1; }
[[ -s "$MODULE_SRC/extra/stcp.ko" ]] || { echo "[FAIL] packaged stcp.ko missing" >&2; exit 1; }

STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP="/root/stcp-kernel-backup-${KREL}-${STAMP}"
mkdir -p "$BACKUP"
cp -a "$CONFIG_TXT" "$BACKUP/config.txt"
[[ -f "$KERNEL_DST" ]] && cp -a "$KERNEL_DST" "$BACKUP/"
[[ -d "$MODULE_DST" ]] && cp -a "$MODULE_DST" "$BACKUP/modules-$KREL"
for f in "/boot/vmlinuz-$KREL" "/boot/System.map-$KREL" "/boot/config-$KREL"; do
    [[ -e "$f" ]] && cp -a "$f" "$BACKUP/"
done

echo "[INFO] Installing complete module tree -> $MODULE_DST"
rm -rf "$MODULE_DST"
mkdir -p "$MODULE_DST"
cp -a "$MODULE_SRC/." "$MODULE_DST/"

echo "[INFO] Installing kernel -> $KERNEL_DST"
install -m0644 "$PKG_DIR/Image" "$KERNEL_DST"
install -m0644 "$PKG_DIR/Image" "/boot/vmlinuz-$KREL"
[[ -f "$PKG_DIR/System.map" ]] && install -m0644 "$PKG_DIR/System.map" "/boot/System.map-$KREL"
[[ -f "$PKG_DIR/kernel.config" ]] && install -m0644 "$PKG_DIR/kernel.config" "/boot/config-$KREL"
"$DEPMOD" -a "$KREL"

TMP_CONFIG="$(mktemp)"
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
kernel=$KERNEL_FILE
# END STCP KERNEL
CFG
cp --no-preserve=ownership,mode,timestamps "$TMP_CONFIG" "${CONFIG_TXT}.new"
mv -f "${CONFIG_TXT}.new" "$CONFIG_TXT"
rm -f "$TMP_CONFIG" "$TMP_CLEAN"
sync

grep -Fxq "kernel=$KERNEL_FILE" "$CONFIG_TXT" || { echo "[FAIL] config.txt activation failed" >&2; exit 1; }
[[ -s "$KERNEL_DST" ]] || { echo "[FAIL] installed kernel missing" >&2; exit 1; }
[[ -s "$MODULE_DST/extra/stcp.ko" ]] || { echo "[FAIL] installed STCP module missing" >&2; exit 1; }

ROLLBACK="/usr/local/sbin/rollback-stcp-kernel-${KREL}.sh"
cat > "$ROLLBACK" <<ROLLBACK_SCRIPT
#!/usr/bin/env bash
set -Eeuo pipefail
[[ "\$(id -u)" == 0 ]] || { echo "Run as root" >&2; exit 1; }
cp -a "$BACKUP/config.txt" "$CONFIG_TXT"
rm -rf "$MODULE_DST"
[[ -d "$BACKUP/modules-$KREL" ]] && cp -a "$BACKUP/modules-$KREL" "$MODULE_DST"
"$DEPMOD" -a "$KREL" || true
sync
echo "Restored boot config/module tree from $BACKUP"
ROLLBACK_SCRIPT
chmod 0755 "$ROLLBACK"

echo "[OK] Raspberry Pi kernel + modules installed"
echo "Kernel release : $KREL"
echo "Kernel image   : $KERNEL_DST"
echo "Modules        : $MODULE_DST"
echo "Backup         : $BACKUP"
echo "Rollback       : sudo $ROLLBACK"
echo "Reboot to activate kernel $KREL"
REMOTE_INSTALL
    chmod 0755 "$tmp/$pkg/install.sh"

    archive="$tmp/${pkg}.tar.gz"
    tar -C "$tmp" -czf "$archive" "$pkg"
    remote_archive="/tmp/${pkg}.tar.gz"

    echo "[INFO] Copying kernel package -> $remote:$remote_archive"
    pnc_note "STCP Raspberry Pi install" "Deploying kernel $krel + modules to $remote"
    pnc_run "STCPv2/Raspberry Pi package upload" scp "${ssh_opts[@]}" "$archive" "$remote:$remote_archive"
    echo "[INFO] Installing Raspberry Pi kernel + modules"
    pnc_run "STCPv2/Raspberry Pi kernel install" ssh "${ssh_opts[@]}" "$remote" "
        set -Eeuo pipefail
        rm -rf '/tmp/$pkg'
        tar -C /tmp -xzf '$remote_archive'
        $sudo_remote bash '/tmp/$pkg/install.sh'
        rm -rf '/tmp/$pkg' '$remote_archive'
    " || {
        echo "[FAIL] Raspberry Pi kernel installation failed" >&2
        return 1
    }

    echo "[OK] Raspberry Pi kernel + STCP module are installed and ready"
    echo "[INFO] Installed kernel release: $krel"
    pnc_note "STCP Raspberry Pi install complete" "Kernel $krel + STCP module installed on $remote"
    if [[ "$reboot" == 1 ]]; then
        echo "[INFO] Rebooting Raspberry Pi"
        pnc_note "STCP Raspberry Pi reboot" "$remote rebooting into kernel $krel"
        ssh "${ssh_opts[@]}" "$remote" "$sudo_remote reboot" >/dev/null 2>&1 || true
    else
        echo "[INFO] Not rebooting (RPI_REBOOT=0). Activate with: ssh $remote '$sudo_remote reboot'"
    fi
}

flash_zephyr() {
    local variant="$1" builddir script ncs
    case "$variant" in
        nrf9151)
            builddir="$ZAPP/build-nrf9151"
            script="$ZAPP/scripts/flash.sh"
            ;;
        ethernet)
            builddir="$ZAPP/build-ethernet"
            script="$ZAPP/scripts/flash-ethernet.sh"
            ;;
        *) echo "[FAIL] Unknown Zephyr variant: $variant" >&2; return 2 ;;
    esac

    [[ -d "$builddir" ]] || {
        echo "[FAIL] Missing Zephyr build directory: $builddir" >&2
        echo "       Build it first with: scripts/build-all.sh zephyr-$variant" >&2
        return 1
    }

    ncs="${NCS_DIR:-$ROOT/zephyr/nordic/ncs-3.3.0}"
    [[ -x "$ncs/.venv/bin/python" ]] || {
        echo "[FAIL] NCS environment missing: $ncs/.venv/bin/python" >&2
        echo "       Set NCS_DIR=/path/to/ncs" >&2
        return 1
    }

    echo "[INFO] Flashing Zephyr variant: $variant"
    pnc_run "STCPv2 Zephyr $variant flash" env NCS_DIR="$ncs" bash "$script"
    echo "[OK] Zephyr $variant flashed"
    pnc_note "STCP Zephyr flash complete" "$variant flashed successfully"
}

cmd="${1:-host}"
case "$cmd" in
    host) install_host ;;
    rpi) install_rpi ;;
    kernel) install_host ;;
    zephyr-nrf9151) flash_zephyr nrf9151 ;;
    zephyr-ethernet) flash_zephyr ethernet ;;
    all)
        install_host
        flash_zephyr "${ZEPHYR_VARIANT:-nrf9151}"
        ;;
    -h|--help) usage ;;
    *) echo "[FAIL] Unknown action: $cmd" >&2; usage >&2; exit 2 ;;
esac
