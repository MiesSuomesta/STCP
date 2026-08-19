#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
KMOD="$ROOT/kernel/module"
ZAPP="$ROOT/zephyr/nordic/application"
OUT="$ROOT/artifacts/build"
JOBS="${JOBS:-$(nproc)}"
STRICT="${STRICT:-0}"
PNC_ENABLE="${PNC_ENABLE:-1}"
PNC_APP="${PNC_APP:-STCPv2 build}"
PNC_MOBILE="${PNC_MOBILE:-0}"
PNC_WRAPPER="${PNC_WRAPPER:-}"
LOCALVERSION=${LOCALVERSION:--stcp}

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

DO_HOST=1
DO_RPI=1
DO_ZEPHYR_NRF9151=1
DO_ZEPHYR_ETHERNET=1

usage() {
    cat <<USAGE
Usage: $(basename "$0") [target ...]

Targets:
  all               Build every target that is available (default)
  host              Linux host kernel module
  rpi               Raspberry Pi ARM64 kernel module
  zephyr            Both Zephyr variants
  zephyr-nrf9151    nRF9151/LTE Zephyr firmware
  zephyr-ethernet   nRF9151 + W5500 Zephyr firmware

Environment:
  JOBS=N             Parallel build jobs (default: nproc)
  RPI_KDIR=PATH      Raspberry Pi kernel tree (default: kernel/raspberry)
  RPI_CONFIG=PATH    Known-good Raspberry Pi kernel config (default: kernel/rpi-working.config)
  RPI_CROSS_COMPILE= Prefix, e.g. aarch64-linux-gnu-
  RPI_TARGET=pi4|pi5 Raspberry Pi board target (default: pi4)
  NCS_DIR=PATH       Nordic Connect SDK tree (default used by app scripts)
  ZEPHYR_SDK_INSTALL_DIR=PATH
  STRICT=1           Treat unavailable target as an error instead of skipping
  PNC_ENABLE=0        Disable pncwrap/pncnote integration (default: enabled)
  PNC_WRAPPER=CMD     Force wrapper command (default: pncwrap, then pncwrapper)
  PNC_MOBILE=1        Also send pncnote notifications to mobile
USAGE
}

if (( $# > 0 )); then
    DO_HOST=0; DO_RPI=0; DO_ZEPHYR_NRF9151=0; DO_ZEPHYR_ETHERNET=0
    for arg in "$@"; do
        case "$arg" in
            all) DO_HOST=1; DO_RPI=1; DO_ZEPHYR_NRF9151=1; DO_ZEPHYR_ETHERNET=1 ;;
            host) DO_HOST=1 ;;
            rpi) DO_RPI=1 ;;
            zephyr) DO_ZEPHYR_NRF9151=1; DO_ZEPHYR_ETHERNET=1 ;;
            zephyr-nrf9151) DO_ZEPHYR_NRF9151=1 ;;
            zephyr-ethernet) DO_ZEPHYR_ETHERNET=1 ;;
            -h|--help) usage; exit 0 ;;
            *) echo "[FAIL] Unknown target: $arg" >&2; usage >&2; exit 2 ;;
        esac
    done
fi

mkdir -p "$OUT"

built=()
skipped=()
failed=()

section() { printf '\n========== %s ==========\n' "$*"; }
skip() {
    echo "[SKIP] $1"
    skipped+=("$1")
    [[ "$STRICT" == 1 ]] && return 1
    return 10
}
run_target() {
    local name="$1"; shift
    local rc=0
    section "$name"
    "$@" || rc=$?
    case "$rc" in
        0) built+=("$name"); return 0 ;;
        10) return 0 ;;
        *) failed+=("$name"); return "$rc" ;;
    esac
}

stage_kernel() {
    local name="$1"
    local dst="$OUT/$name"

    if [[ ! -s "$KMOD/stcp.ko" ]]; then
        echo "[FAIL] $name: build completed without $KMOD/stcp.ko" >&2
        return 1
    fi

    mkdir -p "$dst"
    install -m 0644 "$KMOD/stcp.ko" "$dst/stcp.ko" || return 1
    modinfo "$dst/stcp.ko" > "$dst/modinfo.txt" 2>/dev/null || true
    sha256sum "$dst/stcp.ko" > "$dst/SHA256SUMS" || return 1
}

check_rpi_crypto() {
    local kdir="$1"
    local cfg="$kdir/.config"
    local symvers="$kdir/Module.symvers"
    local missing_cfg=()
    local missing_sym=()
    local item

    if [[ ! -f "$cfg" ]]; then
        echo "[FAIL] rpi: kernel configuration missing: $cfg" >&2
        echo "       Configure/build the Raspberry Pi kernel before building STCP." >&2
        return 1
    fi

    for item in CONFIG_CRYPTO_LIB_CURVE25519 CONFIG_CRYPTO_LIB_CHACHA20POLY1305; do
        grep -Eq "^${item}=(y|m)$" "$cfg" || missing_cfg+=("$item")
    done

    if (( ${#missing_cfg[@]} )); then
        echo "[FAIL] rpi: STCP crypto options are not enabled in $cfg:" >&2
        printf '       - %s\n' "${missing_cfg[@]}" >&2
        echo >&2
        echo "Enable them and rebuild the Raspberry Pi kernel/modules:" >&2
        echo "  cd '$kdir'" >&2
        echo "  scripts/config --module CONFIG_CRYPTO_LIB_CURVE25519" >&2
        echo "  scripts/config --module CONFIG_CRYPTO_LIB_CHACHA20POLY1305" >&2
        echo "  make ARCH=arm64 CROSS_COMPILE=\${RPI_CROSS_COMPILE:-aarch64-linux-gnu-} olddefconfig" >&2
        echo "  make ARCH=arm64 CROSS_COMPILE=\${RPI_CROSS_COMPILE:-aarch64-linux-gnu-} -j\$(nproc) Image modules" >&2
        return 1
    fi

    if [[ ! -s "$symvers" ]]; then
        echo "[FAIL] rpi: $symvers missing/empty; build the Raspberry Pi kernel modules first" >&2
        return 1
    fi

    for item in curve25519 curve25519_generate_public chacha20poly1305_encrypt chacha20poly1305_decrypt; do
        grep -qw "$item" "$symvers" || missing_sym+=("$item")
    done

    if (( ${#missing_sym[@]} )); then
        echo "[FAIL] rpi: kernel Module.symvers does not export STCP crypto symbols:" >&2
        printf '       - %s\n' "${missing_sym[@]}" >&2
        echo >&2
        echo "The .config may have been changed after the last kernel build." >&2
        echo "Rebuild the Raspberry Pi kernel and modules, then retry STCP:" >&2
        echo "  make -C '$kdir' ARCH=arm64 CROSS_COMPILE=\${RPI_CROSS_COMPILE:-aarch64-linux-gnu-} -j\$(nproc) Image modules" >&2
        return 1
    fi

    echo "[ OK ] rpi crypto config + Module.symvers"
}

build_host() {
    [[ -d "/lib/modules/$(uname -r)/build" ]] || { skip "host: /lib/modules/$(uname -r)/build missing"; return $?; }
    make -C "$KMOD" LOCALVERSION=="$LOCALVERSION" clean >/dev/null || true
    if ! pnc_run "STCPv2 host module build" make -C "$KMOD" LOCALVERSION=="$LOCALVERSION" PLATFORM=host JOBS="$JOBS" module; then
        echo "[FAIL] host: kernel module build failed" >&2
        return 1
    fi
    stage_kernel kernel-host || return 1
}

check_rpi_boot_config() {
    local cfg="$1"
    local target="$2"
    local missing=()
    local item

    [[ -f "$cfg" ]] || {
        echo "[FAIL] rpi: boot config missing: $cfg" >&2
        return 1
    }

    # These are required for the Raspberry Pi 4 boot/storage/serial path.
    if [[ "$target" == "pi4" ]]; then
        for item in \
            CONFIG_ARCH_BCM2835 \
            CONFIG_SERIAL_AMBA_PL011 \
            CONFIG_SERIAL_AMBA_PL011_CONSOLE \
            CONFIG_MMC_SDHCI_IPROC \
            CONFIG_MMC_BCM2835; do
            grep -Fxq "${item}=y" "$cfg" || missing+=("$item")
        done

        # VC4 may be built-in or modular; absence is suspicious for our known-good Pi 4 config.
        grep -Eq '^CONFIG_DRM_VC4=(y|m)$' "$cfg" || missing+=("CONFIG_DRM_VC4")
    fi

    if (( ${#missing[@]} )); then
        echo "[FAIL] rpi: boot-critical Raspberry Pi options missing from $cfg:" >&2
        printf '       - %s\n' "${missing[@]}" >&2
        return 1
    fi

    echo "[ OK ] rpi boot-critical config ($target)"
}

build_rpi() {
    local kdir="${RPI_KDIR:-$ROOT/kernel/raspberry}"
    local rpi_config="${RPI_CONFIG:-$ROOT/kernel/rpi-working.config}"
    local cross="${RPI_CROSS_COMPILE:-aarch64-linux-gnu-}"
    local target="${RPI_TARGET:-pi4}"
    local stage="$OUT/kernel-rpi"
    local rootfs="$stage/rootfs"
    local boot="$stage/boot-payload"
    local krel kimage vermagic target_dtb target_dtb_rel target_dtb_path overlay_src
    local -a dtb_roots=()

    case "$target" in
        pi4) target_dtb="bcm2711-rpi-4-b.dtb" ;;
        pi5) target_dtb="bcm2712-rpi-5-b.dtb" ;;
        *) echo "[FAIL] rpi: RPI_TARGET must be pi4 or pi5" >&2; return 1 ;;
    esac

    [[ -d "$kdir" && -f "$kdir/Makefile" ]] || {
        skip "rpi: Raspberry Pi kernel tree missing: $kdir"
        return $?
    }
    [[ -f "$rpi_config" ]] || {
        echo "[FAIL] rpi: known-good Raspberry Pi config missing: $rpi_config" >&2
        echo "       Set RPI_CONFIG=/path/to/rpi-working.config if needed." >&2
        return 1
    }

    echo "[INFO] Restoring known-good Raspberry Pi config: $rpi_config"
    install -m 0644 "$rpi_config" "$kdir/.config"

    echo "[INFO] Normalizing Raspberry Pi config with ARCH=arm64"
    if ! pnc_run "STCPv2/Raspberry Pi olddefconfig" make -C "$kdir" LOCALVERSION=="$LOCALVERSION" ARCH=arm64 CROSS_COMPILE="$cross" olddefconfig; then
        echo "[FAIL] rpi: ARCH=arm64 olddefconfig failed" >&2
        return 1
    fi

    check_rpi_boot_config "$kdir/.config" "$target" || return 1

    echo "[INFO] Building Raspberry Pi kernel + in-tree modules + DTBs ($target)"
    if ! pnc_run "STCPv2/Raspberry Pi kernel build" make -C "$kdir" LOCALVERSION=="$LOCALVERSION" ARCH=arm64 CROSS_COMPILE="$cross" -j"$JOBS" Image modules dtbs; then
        echo "[FAIL] rpi: kernel/Image/modules/dtbs build failed" >&2
        return 1
    fi

    target_dtb_rel="broadcom/$target_dtb"
    target_dtb_path="$kdir/arch/arm64/boot/dts/$target_dtb_rel"
    if [[ ! -f "$target_dtb_path" ]]; then
        echo "[INFO] Building target DTB explicitly: $target_dtb"
        pnc_run "STCPv2/Raspberry Pi target DTB" make -C "$kdir" LOCALVERSION=="$LOCALVERSION" ARCH=arm64 CROSS_COMPILE="$cross" -j"$JOBS" "$target_dtb_rel" || \
        pnc_run "STCPv2/Raspberry Pi target DTB" make -C "$kdir" LOCALVERSION=="$LOCALVERSION" ARCH=arm64 CROSS_COMPILE="$cross" -j"$JOBS" "arch/arm64/boot/dts/$target_dtb_rel" || true
    fi

    check_rpi_crypto "$kdir" || return 1

    krel="$(make -s -C "$kdir" ARCH=arm64 CROSS_COMPILE="$cross" kernelrelease)"
    kimage="$kdir/arch/arm64/boot/Image"
    [[ -n "$krel" ]] || { echo "[FAIL] rpi: could not determine kernel release" >&2; return 1; }
    [[ -s "$kimage" ]] || { echo "[FAIL] rpi: kernel Image missing: $kimage" >&2; return 1; }
    [[ -s "$kdir/System.map" ]] || { echo "[FAIL] rpi: System.map missing" >&2; return 1; }
    [[ -s "$kdir/Module.symvers" ]] || { echo "[FAIL] rpi: Module.symvers missing" >&2; return 1; }

    mapfile -t dtb_roots < <(find "$kdir/arch" -type d -path '*/boot/dts' -print | sort -u)
    (( ${#dtb_roots[@]} > 0 )) || { echo "[FAIL] rpi: no boot/dts roots found" >&2; return 1; }
    target_dtb_path=""
    for root in "${dtb_roots[@]}"; do
        target_dtb_path="$(find "$root" -type f -name "$target_dtb" -print -quit)"
        [[ -n "$target_dtb_path" ]] && break
    done
    [[ -n "$target_dtb_path" ]] || { echo "[FAIL] rpi: target DTB missing after build: $target_dtb" >&2; return 1; }

    echo "[INFO] Building STCP module against Raspberry Pi kernel $krel"
    LOCALVERSION="${LOCALVERSION}" \
       make -C "$KMOD" clean >/dev/null || true
    if ! pnc_run "STCPv2/Raspberry Pi STCP module build" make -C "$KMOD" LOCALVERSION=="$LOCALVERSION" PLATFORM=rpi KDIR="$kdir" CROSS_COMPILE="$cross" JOBS="$JOBS" module; then
        echo "[FAIL] rpi: STCP kernel module build failed" >&2
        return 1
    fi
    [[ -s "$KMOD/stcp.ko" ]] || { echo "[FAIL] rpi: stcp.ko missing after build" >&2; return 1; }

    vermagic="$(modinfo -F vermagic "$KMOD/stcp.ko" 2>/dev/null | awk '{print $1}' || true)"
    if [[ -n "$vermagic" && "$vermagic" != "$krel" ]]; then
        echo "[FAIL] rpi: stcp.ko vermagic=$vermagic, kernel release=$krel" >&2
        return 1
    fi

    echo "[INFO] Staging complete Raspberry Pi boot/module set"
    rm -rf "$stage"
    mkdir -p "$rootfs" "$boot/dtbs" "$boot/overlays"
    if ! pnc_run "STCPv2/Raspberry Pi modules staging" make -C "$kdir" LOCALVERSION=="$LOCALVERSION" ARCH=arm64 CROSS_COMPILE="$cross" \
        INSTALL_MOD_PATH="$rootfs" DEPMOD=true modules_install; then
        echo "[FAIL] rpi: modules_install staging failed" >&2
        return 1
    fi

    mkdir -p "$rootfs/lib/modules/$krel/extra"
    install -m 0644 "$KMOD/stcp.ko" "$rootfs/lib/modules/$krel/extra/stcp.ko"
    find "$rootfs/lib/modules/$krel" -maxdepth 1 -type l \( -name build -o -name source \) -delete 2>/dev/null || true

    install -m 0644 "$kimage" "$boot/Image"
    install -m 0644 "$kdir/System.map" "$boot/System.map"
    install -m 0644 "$kdir/.config" "$boot/config"
    install -m 0644 "$kdir/Module.symvers" "$boot/Module.symvers"

    for root in "${dtb_roots[@]}"; do
        while IFS= read -r -d '' dtb; do
            rel="${dtb#$root/}"
            mkdir -p "$boot/dtbs/$(dirname "$rel")"
            install -m 0644 "$dtb" "$boot/dtbs/$rel"
        done < <(find "$root" -type f -name '*.dtb' -print0)
    done

    overlay_src=""
    for root in "${dtb_roots[@]}"; do
        if [[ -d "$root/overlays" ]] && find "$root/overlays" -maxdepth 1 -type f -name '*.dtbo' -print -quit | grep -q .; then
            overlay_src="$root/overlays"
            break
        fi
    done
    if [[ -n "$overlay_src" ]]; then
        while IFS= read -r -d '' overlay; do
            install -m 0644 "$overlay" "$boot/overlays/$(basename "$overlay")"
        done < <(find "$overlay_src" -maxdepth 1 -type f \( -name '*.dtbo' -o -name '*.dtb' -o -name README \) -print0)
    fi

    find "$boot/dtbs" -type f -name "$target_dtb" -print -quit | grep -q . || {
        echo "[FAIL] rpi: target DTB not staged: $target_dtb" >&2
        return 1
    }

    printf '%s\n' "$krel" > "$stage/KERNEL_RELEASE"
    printf '%s\n' "$target" > "$stage/TARGET"
    printf '%s\n' "$target_dtb" > "$stage/TARGET_DTB"
    mkdir -p "$stage/module"
    install -m 0644 "$KMOD/stcp.ko" "$stage/module/stcp.ko"
    modinfo "$KMOD/stcp.ko" > "$stage/module/modinfo.txt" 2>/dev/null || true

    cat > "$stage/manifest.txt" <<MANIFEST
kernel_release=$krel
target=$target
target_dtb=$target_dtb
stcp_vermagic=$vermagic
kernel_image=yes
system_map=yes
kernel_config=yes
module_symvers=yes
modules=yes
dtbs=yes
overlays=packaged_if_available_else_copied_from_target
initramfs=generated_on_target
config_baseline=$rpi_config
boot_layout=dedicated_kernel_filename
MANIFEST

    (
        cd "$stage"
        find . -type f ! -name SHA256SUMS -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS
    )

    echo "[OK] Raspberry Pi kernel + modules + boot payload staged: $stage"
    echo "[INFO] kernel release: $krel"
    echo "[INFO] target DTB    : $target_dtb"
}

ncs_available() {
    local ncs="${NCS_DIR:-$ROOT/zephyr/nordic/ncs-3.3.0}"
    [[ -x "$ncs/.venv/bin/python" ]]
}

stage_zephyr() {
    local name="$1" builddir="$2" dst="$OUT/$1"
    rm -rf "$dst"
    mkdir -p "$dst"
    local found=0 rel
    while IFS= read -r -d '' f; do
        found=1
        rel="${f#"$builddir"/}"
        mkdir -p "$dst/$(dirname "$rel")"
        cp -a "$f" "$dst/$rel"
    done < <(find "$builddir" -type f \( -name 'merged.hex' -o -name 'zephyr.hex' -o -name 'zephyr.bin' -o -name 'zephyr.elf' -o -name 'app_signed.hex' \) -print0 2>/dev/null)
    if (( found == 0 )); then
        echo "[WARN] Build succeeded but no standard firmware artifact was found under $builddir"
    else
        (cd "$dst" && find . -type f ! -name SHA256SUMS -print0 | sort -z | xargs -0 sha256sum > SHA256SUMS)
    fi
}

build_zephyr_nrf9151() {
    ncs_available || { skip "zephyr-nrf9151: NCS Python environment missing; set NCS_DIR"; return $?; }
    if ! pnc_run "STCPv2 Zephyr nRF9151 build" env NCS_DIR="${NCS_DIR:-$ROOT/zephyr/nordic/ncs-3.3.0}" bash "$ZAPP/scripts/build.sh"; then
        echo "[FAIL] zephyr-nrf9151: firmware build failed" >&2
        return 1
    fi
    stage_zephyr zephyr-nrf9151 "$ZAPP/build-nrf9151" || return 1
}

build_zephyr_ethernet() {
    ncs_available || { skip "zephyr-ethernet: NCS Python environment missing; set NCS_DIR"; return $?; }
    if ! pnc_run "STCPv2 Zephyr Ethernet build" env NCS_DIR="${NCS_DIR:-$ROOT/zephyr/nordic/ncs-3.3.0}" bash "$ZAPP/scripts/build-ethernet.sh"; then
        echo "[FAIL] zephyr-ethernet: firmware build failed" >&2
        return 1
    fi
    stage_zephyr zephyr-ethernet "$ZAPP/build-ethernet" || return 1
}

rc=0
if (( DO_HOST )); then run_target "host" build_host || rc=1; fi
if (( DO_RPI )); then run_target "rpi" build_rpi || rc=1; fi
if (( DO_ZEPHYR_NRF9151 )); then run_target "zephyr-nrf9151" build_zephyr_nrf9151 || rc=1; fi
if (( DO_ZEPHYR_ETHERNET )); then run_target "zephyr-ethernet" build_zephyr_ethernet || rc=1; fi

section "SUMMARY"
printf 'Built:\n'
if (( ${#built[@]} )); then printf '  - %s\n' "${built[@]}"; else echo '  (none)'; fi
printf 'Skipped:\n'
if (( ${#skipped[@]} )); then printf '  - %s\n' "${skipped[@]}"; else echo '  (none)'; fi
printf 'Failed:\n'
if (( ${#failed[@]} )); then printf '  - %s\n' "${failed[@]}"; else echo '  (none)'; fi
echo "Artifacts: $OUT"

summary="Built: ${built[*]:-(none)}; Skipped: ${skipped[*]:-(none)}; Failed: ${failed[*]:-(none)}"
if (( ${#failed[@]} )); then
    pnc_note "STCPv2 build FAILED" "$summary"
elif [[ "$STRICT" == 1 && ${#skipped[@]} -ne 0 ]]; then
    pnc_note "STCPv2 build incomplete" "$summary"
else
    pnc_note "STCPv2 build complete" "$summary"
fi

(( ${#failed[@]} == 0 )) || exit 1
[[ "$STRICT" != 1 || ${#skipped[@]} == 0 ]] || exit 1
exit "$rc"
