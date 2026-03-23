#!/usr/bin/env bash
set -euo pipefail

# Kernel version we target
KVER="${KVER:-$(uname -r)}"

# Where your module build happens (adjust if needed)
# Expectation: `make` produces stcp.ko somewhere predictable
OUTDIR="deb/stcp_pkg"
mkdir -p "${OUTDIR}/scripts"

echo "[STCP] Building module for KVER=${KVER}"

# 1) Build the module (adjust to your actual build command)
# Example assumes you're in module dir with a Makefile:
make clean || true
make -j"$(nproc)"

# 2) Locate stcp.ko
# Adjust this path if your build outputs elsewhere.
cp -v "./kmod/stcp_rust.ko" "${OUTDIR}/stcp.ko"

# 3) Optional modprobe config
cat > "${OUTDIR}/stcp.conf" <<'EOF'
# /etc/modprobe.d/stcp.conf
# options stcp debug=1
EOF

# 4) README.Debian
cat > "${OUTDIR}/README.Debian" <<EOF
STCP kernel module package (prebuilt)

This package installs stcp.ko for:
  ${KVER}

After install:
  sudo modprobe stcp
  dmesg | tail -n 200

If Secure Boot is enabled, unsigned modules may fail to load.
EOF

# 5) postinst / prerm scripts
cat > "${OUTDIR}/scripts/postinst" <<'EOF'
#!/bin/sh
set -e

KVER="${KVER:-__KVER__}"

echo "STCP: running depmod for ${KVER}"
depmod -a "${KVER}" || depmod -a || true

# Don't hard-fail if running kernel differs or module can't load
if [ "$(uname -r)" = "${KVER}" ]; then
  echo "STCP: attempting to modprobe stcp (non-fatal if it fails)"
  modprobe stcp || true
else
  echo "STCP: installed for ${KVER} but running $(uname -r); not auto-loading."
fi

exit 0
EOF

cat > "${OUTDIR}/scripts/prerm" <<'EOF'
#!/bin/sh
set -e

KVER="${KVER:-__KVER__}"

# Best-effort unload (non-fatal)
if lsmod | grep -q '^stcp '; then
  echo "STCP: attempting to unload module"
  modprobe -r stcp || true
fi

echo "STCP: running depmod for ${KVER}"
depmod -a "${KVER}" || depmod -a || true

exit 0
EOF

chmod 0755 "${OUTDIR}/scripts/postinst" "${OUTDIR}/scripts/prerm"

# 6) Patch __KVER__ placeholders into scripts so they work without env
sed -i "s/__KVER__/${KVER}/g" "${OUTDIR}/scripts/postinst" "${OUTDIR}/scripts/prerm"

echo "[STCP] Prepared package payload in ${OUTDIR}"
ls -lah "${OUTDIR}"
