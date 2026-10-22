#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT="$ROOT/artifacts/build"
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
Usage: $(basename "$0") [host|rpi|kernel|all] [--check]

Default: host

  --host               Install staged host kernel module on this machine
  --rpi                Install staged Raspberry Pi kernel + module tree over SSH
  --kernel             Backwards-compatible alias for 'host'
  --all                Install host and Raspberry Pi protocol modules

Environment:
  SUDO=command                      local sudo command (default: sudo)
  RPI_HOST=host                     Raspberry Pi SSH host (default: raspi)
  RPI_USER=user                     Raspberry Pi SSH user (default: pi)
  RPI_SUDO=command                  remote sudo command (default: sudo)
  RPI_SSH_OPTS="..."               extra options for ssh/scp
  RPI_REBOOT=1                     reboot Raspberry after successful install (default: 0)
  PNC_ENABLE=0                      disable pncwrap/pncnote integration
  PNC_WRAPPER=CMD                   force pncwrap/pncwrapper command
  PNC_MOBILE=1                      also send pncnote notifications to mobile

--check validates local staged artifacts without installing.
Build Linux artifacts with STCP/scripts/build-all.sh (or scripts/build.sh --linux).
USAGE
}

cleanup_module_users() {
	echo "[NOP] Cleaning module users...."
}

restart_module_users() {
	echo "[NOP] Restart module users...."
}

install_host() {
    local src release vermagic
    release="$(uname -r)"
    src="$OUT/kernel-host/stcp.ko"

    [[ -s "$src" ]] || {
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

    if ((CHECK_ONLY)); then echo "[OK] Host staged module: $src"; return 0; fi

    echo "[INFO] Installing host module for kernel $release"
    cleanup_module_users
    pnc_note "STCP host install" "Installing STCP module for kernel $release"
    if grep -q '^stcp ' /proc/modules 2>/dev/null; then
        "$SUDO" modprobe -r stcp
    fi
    "$SUDO" modprobe libcurve25519 || true
    "$SUDO" modprobe libchacha20poly1305 || true
    "$SUDO" install -D -m 0644 "$src" "/lib/modules/$release/extra/stcp.ko"
    "$SUDO" depmod -a "$release"
    "$SUDO" modprobe stcp
    restart_module_users

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
    local host user remote sudo_remote reboot krel target target_dtb local_module vermagic
    local pkg tmp archive remote_archive
    local -a ssh_opts=()

    host="${RPI_HOST:-raspi}"
    user="${RPI_USER:-pi}"
    sudo_remote="${RPI_SUDO:-sudo}"
    reboot="${RPI_REBOOT:-0}"
    remote="${user}@${host}"

    [[ -s "$stage/KERNEL_RELEASE" ]] || { echo "[FAIL] Missing $stage/KERNEL_RELEASE" >&2; return 1; }
    [[ -s "$stage/TARGET" ]] || { echo "[FAIL] Missing $stage/TARGET" >&2; return 1; }
    [[ -s "$stage/TARGET_DTB" ]] || { echo "[FAIL] Missing $stage/TARGET_DTB" >&2; return 1; }
    [[ -s "$stage/boot-payload/Image" ]] || {
        echo "[FAIL] Missing staged Raspberry Pi kernel: $stage/boot-payload/Image" >&2
        echo "       Build first with: scripts/build-all.sh rpi" >&2
        return 1
    }

    krel="$(<"$stage/KERNEL_RELEASE")"
    target="$(<"$stage/TARGET")"
    target_dtb="$(<"$stage/TARGET_DTB")"
    [[ $krel =~ ^[a-zA-Z0-9._+-]+$ && $target =~ ^[a-zA-Z0-9._+-]+$ &&
       $target_dtb =~ ^[a-zA-Z0-9._+-]+\.dtb$ ]] || {
        echo '[FAIL] Invalid staged kernel/target metadata' >&2; return 1;
    }
    [[ $krel != . && $krel != .. && $target != . && $target != .. ]] || {
        echo "[FAIL] Invalid staged metadata component" >&2; return 1;
    }
    [[ -d "$stage/rootfs/lib/modules/$krel" ]] || {
        echo "[FAIL] Missing staged module tree: $stage/rootfs/lib/modules/$krel" >&2
        return 1
    }
    local_module="$stage/rootfs/lib/modules/$krel/extra/stcp.ko"
    [[ -s "$local_module" ]] || { echo "[FAIL] Missing staged STCP module: $local_module" >&2; return 1; }
    find "$stage/boot-payload/dtbs" -type f -name "$target_dtb" -print -quit | grep -q . || {
        echo "[FAIL] Missing staged target DTB: $target_dtb" >&2
        return 1
    }

    vermagic="$(modinfo -F vermagic "$local_module" 2>/dev/null | awk '{print $1}' || true)"
    if [[ -n "$vermagic" && "$vermagic" != "$krel" ]]; then
        echo "[FAIL] staged STCP vermagic=$vermagic, staged kernel=$krel" >&2
        return 1
    fi

    for file in System.map config Module.symvers; do
        [[ -s "$stage/boot-payload/$file" ]] || {
            echo "[FAIL] Missing boot metadata: $stage/boot-payload/$file" >&2; return 1;
        }
    done
    if ((CHECK_ONLY)); then echo "[OK] Pi staged boot set: $krel"; return 0; fi

    if [[ -n "${RPI_SSH_OPTS:-}" ]]; then
        read -r -a ssh_opts <<< "$RPI_SSH_OPTS"
    else
        ssh_opts=(-o BatchMode=yes -o ConnectTimeout=10 -o StrictHostKeyChecking=accept-new)
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
    pkg="stcp-rpi-install-${target}-${krel}"
    mkdir -p "$tmp/$pkg"
    cp -a "$stage/rootfs" "$tmp/$pkg/rootfs"
    cp -a "$stage/boot-payload" "$tmp/$pkg/boot-payload"
    cp -a "$stage/KERNEL_RELEASE" "$tmp/$pkg/KERNEL_RELEASE"
    cp -a "$stage/TARGET" "$tmp/$pkg/TARGET"
    cp -a "$stage/TARGET_DTB" "$tmp/$pkg/TARGET_DTB"
    [[ -f "$stage/manifest.txt" ]] && cp -a "$stage/manifest.txt" "$tmp/$pkg/manifest.txt"

    cat > "$tmp/$pkg/install.sh" <<'REMOTE_INSTALL'
#!/usr/bin/env bash
set -Eeuo pipefail
trap 'rc=$?; echo "[FAIL] install.sh line=$LINENO command=$BASH_COMMAND rc=$rc" >&2' ERR

log() { echo "[INFO] $*"; }
die() { echo "[FAIL] $*" >&2; exit 1; }

[[ "$(id -u)" == 0 ]] || die "Run installer as root"

PKG_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOTFS="$PKG_DIR/rootfs"
BOOT_PAYLOAD="$PKG_DIR/boot-payload"
KREL="$(<"$PKG_DIR/KERNEL_RELEASE")"
TARGET="$(<"$PKG_DIR/TARGET")"
TARGET_DTB="$(<"$PKG_DIR/TARGET_DTB")"
KERNEL_FILE="kernel-stcp-${KREL}.img"
INITRAMFS_FILE="initramfs-stcp-${KREL}"

if [[ -d /boot/firmware && -f /boot/firmware/config.txt ]]; then
    BOOT_DIR=/boot/firmware
elif [[ -f /boot/config.txt ]]; then
    BOOT_DIR=/boot
else
    die "Raspberry Pi config.txt not found"
fi

CONFIG_TXT="$BOOT_DIR/config.txt"
CMDLINE_TXT="$BOOT_DIR/cmdline.txt"
MODULE_SRC="$ROOTFS/lib/modules/$KREL"
MODULE_DST="/lib/modules/$KREL"

[[ -d "$MODULE_SRC" ]] || die "Packaged modules missing: $MODULE_SRC"
[[ -s "$MODULE_SRC/extra/stcp.ko" ]] || die "Packaged STCP module missing"
[[ -s "$BOOT_PAYLOAD/Image" ]] || die "Packaged kernel Image missing"
[[ -s "$BOOT_PAYLOAD/System.map" ]] || die "Packaged System.map missing"
[[ -s "$BOOT_PAYLOAD/config" ]] || die "Packaged kernel config missing"
[[ -d "$BOOT_PAYLOAD/dtbs" ]] || die "Packaged DTBs missing"
[[ -d "$BOOT_PAYLOAD/overlays" ]] || die "Packaged overlays directory missing"
[[ -f "$CMDLINE_TXT" ]] || die "Missing target cmdline.txt: $CMDLINE_TXT"
find "$BOOT_PAYLOAD/dtbs" -type f -name "$TARGET_DTB" -print -quit | grep -q . || die "Packaged target DTB missing: $TARGET_DTB"

DEPMOD="$(command -v depmod 2>/dev/null || true)"; [[ -n "$DEPMOD" ]] || DEPMOD=/usr/sbin/depmod
UPDATE_INITRAMFS="$(command -v update-initramfs 2>/dev/null || true)"; [[ -n "$UPDATE_INITRAMFS" ]] || UPDATE_INITRAMFS=/usr/sbin/update-initramfs
[[ -x "$DEPMOD" ]] || die "depmod missing"
[[ -x "$UPDATE_INITRAMFS" ]] || die "update-initramfs missing"

STAMP="$(date +%Y%m%d-%H%M%S)"
BACKUP="/root/stcp-kernel-backup-${KREL}-${STAMP}"
mkdir -p "$BACKUP"

log "Backing up current boot configuration and conflicting files -> $BACKUP"
cp -a "$CONFIG_TXT" "$BACKUP/config.txt"
cp -a "$CMDLINE_TXT" "$BACKUP/cmdline.txt"
[[ -e "$BOOT_DIR/$KERNEL_FILE" ]] && cp -a "$BOOT_DIR/$KERNEL_FILE" "$BACKUP/"
[[ -e "$BOOT_DIR/$INITRAMFS_FILE" ]] && cp -a "$BOOT_DIR/$INITRAMFS_FILE" "$BACKUP/"
[[ -d "$MODULE_DST" ]] && cp -a "$MODULE_DST" "$BACKUP/modules-$KREL"
for f in "/boot/vmlinuz-$KREL" "/boot/System.map-$KREL" "/boot/config-$KREL" "/boot/initrd.img-$KREL"; do
    [[ -e "$f" ]] && cp -a "$f" "$BACKUP/"
done

log "Installing complete kernel module tree"
rm -rf "$MODULE_DST"
mkdir -p "$MODULE_DST"
cp -a "$MODULE_SRC/." "$MODULE_DST/"
"$DEPMOD" -a "$KREL"

log "Installing standard /boot kernel metadata"
install -m0644 "$BOOT_PAYLOAD/Image" "/boot/vmlinuz-$KREL"
install -m0644 "$BOOT_PAYLOAD/System.map" "/boot/System.map-$KREL"
install -m0644 "$BOOT_PAYLOAD/config" "/boot/config-$KREL"

log "Generating initramfs on Raspberry Pi"
rm -f "/boot/initrd.img-$KREL"
"$UPDATE_INITRAMFS" -c -k "$KREL"
[[ -s "/boot/initrd.img-$KREL" ]] || die "initramfs generation failed"

log "Installing STCP kernel alongside working Raspberry Pi kernel"
install -m0644 "$BOOT_PAYLOAD/Image" "$BOOT_DIR/$KERNEL_FILE"
install -m0644 "/boot/initrd.img-$KREL" "$BOOT_DIR/$INITRAMFS_FILE"
[[ -s "$BOOT_DIR/$KERNEL_FILE" ]] || die "Installed kernel is empty: $BOOT_DIR/$KERNEL_FILE"
[[ -s "$BOOT_DIR/$INITRAMFS_FILE" ]] || die "Installed initramfs is empty: $BOOT_DIR/$INITRAMFS_FILE"

# Keep the currently working Raspberry Pi DTBs and overlays in place.
# They are the rescue baseline. The STCP kernel is selected only by kernel=.
[[ -f "$BOOT_DIR/$TARGET_DTB" ]] || die "Working target DTB missing from boot filesystem: $BOOT_DIR/$TARGET_DTB"
[[ -d "$BOOT_DIR/overlays" ]] || die "Working overlays directory missing: $BOOT_DIR/overlays"

log "Activating STCP kernel in config.txt"
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
kernel=${KERNEL_FILE}
initramfs ${INITRAMFS_FILE} followkernel
# END STCP KERNEL
CFG
cp --no-preserve=ownership,mode,timestamps "$TMP_CONFIG" "${CONFIG_TXT}.new"
mv -f "${CONFIG_TXT}.new" "$CONFIG_TXT"
rm -f "$TMP_CONFIG" "$TMP_CLEAN"
sync

grep -Fxq "kernel=${KERNEL_FILE}" "$CONFIG_TXT" || die "config.txt kernel activation failed"
grep -Fxq "initramfs ${INITRAMFS_FILE} followkernel" "$CONFIG_TXT" || die "config.txt initramfs activation failed"

ROLLBACK="/usr/local/sbin/rollback-stcp-kernel-${KREL}.sh"
cat > "$ROLLBACK" <<ROLLBACK_SCRIPT
#!/usr/bin/env bash
set -Eeuo pipefail
[[ "\$(id -u)" == 0 ]] || { echo "Run as root" >&2; exit 1; }
cp -a "$BACKUP/config.txt" "$CONFIG_TXT"
rm -f "$BOOT_DIR/$KERNEL_FILE" "$BOOT_DIR/$INITRAMFS_FILE"
if [[ -f "$BACKUP/$KERNEL_FILE" ]]; then
    cp -a "$BACKUP/$KERNEL_FILE" "$BOOT_DIR/$KERNEL_FILE"
fi
if [[ -f "$BACKUP/$INITRAMFS_FILE" ]]; then
    cp -a "$BACKUP/$INITRAMFS_FILE" "$BOOT_DIR/$INITRAMFS_FILE"
fi
rm -rf "$MODULE_DST"
if [[ -d "$BACKUP/modules-$KREL" ]]; then
    cp -a "$BACKUP/modules-$KREL" "$MODULE_DST"
fi
"$DEPMOD" -a "$KREL" || true
sync
echo "Restored previous Raspberry Pi boot configuration from $BACKUP"
echo "Reboot when ready."
ROLLBACK_SCRIPT
chmod 0755 "$ROLLBACK"
sync

cat <<SUMMARY

[OK] Raspberry Pi kernel + STCP installation complete
Kernel release : $KREL
Target         : $TARGET
Target DTB     : $TARGET_DTB
Boot directory : $BOOT_DIR
Boot kernel    : $BOOT_DIR/$KERNEL_FILE
Boot initramfs : $BOOT_DIR/$INITRAMFS_FILE
Modules        : $MODULE_DST
Initramfs      : /boot/initrd.img-$KREL
Backup         : $BACKUP
Rollback       : sudo $ROLLBACK

Active config block:
SUMMARY
grep -A6 -B1 'BEGIN STCP KERNEL' "$CONFIG_TXT"
REMOTE_INSTALL
    chmod 0755 "$tmp/$pkg/install.sh"

    archive="$tmp/${pkg}.tar.gz"
    tar -C "$tmp" -czf "$archive" "$pkg"
    remote_archive="/tmp/${pkg}.tar.gz"

    echo "[INFO] Copying Raspberry Pi kernel package -> $remote:$remote_archive"
    pnc_note "STCP Raspberry Pi install" "Deploying $target kernel $krel + STCP to $remote"
    pnc_run "STCPv2/Raspberry Pi package upload" scp "${ssh_opts[@]}" "$archive" "$remote:$remote_archive" || return 1

    echo "[INFO] Installing Raspberry Pi isolated boot set"
    pnc_run "STCPv2/Raspberry Pi kernel install" ssh "${ssh_opts[@]}" "$remote" "
        set -Eeuo pipefail
        rm -rf '/tmp/$pkg'
        tar -C /tmp -xzf '$remote_archive'
        $sudo_remote bash '/tmp/$pkg/install.sh'
        rm -rf '/tmp/$pkg' '$remote_archive'
    " || {
        echo "[FAIL] Raspberry Pi kernel installation failed" >&2
        pnc_note "STCP Raspberry Pi install FAILED" "Kernel $krel installation failed on $remote"
        return 1
    }

    echo "[OK] Raspberry Pi kernel + STCP boot set installed and ready"
    echo "[INFO] Kernel release: $krel"
    echo "[INFO] Boot kernel   : kernel-stcp-${krel}.img"
    pnc_note "STCP Raspberry Pi install complete" "Kernel $krel + STCP installed on $remote"

    if [[ "$reboot" == 1 ]]; then
        echo "[INFO] Rebooting Raspberry Pi"
        pnc_note "STCP Raspberry Pi reboot" "$remote rebooting into kernel $krel"
        ssh "${ssh_opts[@]}" "$remote" "$sudo_remote reboot" >/dev/null 2>&1 || true
    else
        echo "[INFO] Not rebooting (RPI_REBOOT=0)."
        echo "[INFO] Activate with: ssh $remote '$sudo_remote reboot'"
    fi
}

CHECK_ONLY=0
cmd=host
seen_target=0
for flag in "$@"; do
    case "$flag" in
        -h|--help) usage; exit 0 ;;
        --check) CHECK_ONLY=1 ;;
        host|rpi|kernel|all|--host|--rpi|--kernel|--all)
            ((seen_target == 0)) || { echo '[FAIL] Only one Linux target/group allowed' >&2; exit 2; }
            cmd="${flag#--}"; seen_target=1 ;;
        *) echo "[FAIL] Unknown action: $flag" >&2; usage >&2; exit 2 ;;
    esac
done
case "$cmd" in
    host|kernel) install_host ;;
    rpi) install_rpi ;;
    all)
        if ((CHECK_ONLY)); then install_host; install_rpi
        else
            bash "${BASH_SOURCE[0]}" all --check
            install_host
            install_rpi
        fi
        ;;
esac
