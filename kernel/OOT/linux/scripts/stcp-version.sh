#!/usr/bin/env bash
set -euo pipefail

# Default header path (can be overridden with --header)
HEADER_DEFAULT="kmod/include/stcp/the_version_include.h"

usage() {
  cat <<'EOF'
Usage: stcp-version.sh [OPTIONS]

Options:
  --header PATH         Path to the_version_include.h
  --git-dir PATH        Git repository root (default: auto-detect from cwd)
  --date "..."          Override build date (default: current UTC)
  --sha "..."           Override git sha (default: from git, else nogit)

Output mode (choose one):
  --print-kmod-version  Print MODULE_VERSION string, e.g. 0.0.1-beta+abcd1234.20260121-083300Z
  --print-deb-version   Print Debian version string, e.g. 0.0.1~beta+gitabcd1234.20260121-083300Z
  --print-defines       Print -D defines for compiler, e.g. -DSTCP_GIT_SHA="abcd1234" -DSTCP_BUILD_DATE="20260121-083300Z"

Other:
  --print-all           Print key=value lines (STCP_VERSION, STCP_GIT_SHA, STCP_BUILD_DATE, KMOD_VERSION, DEB_VERSION)
  -h, --help            Show this help

Examples:
  ./scripts/stcp-version.sh --print-kmod-version
  ./scripts/stcp-version.sh --print-deb-version
  make EXTRA_CFLAGS="$(./scripts/stcp-version.sh --print-defines)"
EOF
}

HEADER="$HEADER_DEFAULT"
GIT_DIR=""
OVERRIDE_DATE=""
OVERRIDE_SHA=""
MODE=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --header) HEADER="$2"; shift 2;;
    --git-dir) GIT_DIR="$2"; shift 2;;
    --date) OVERRIDE_DATE="$2"; shift 2;;
    --sha) OVERRIDE_SHA="$2"; shift 2;;
    --print-kmod-version|--print-deb-version|--print-defines|--print-all)
      MODE="$1"; shift 1;;
    -h|--help) usage; exit 0;;
    *) echo "Unknown arg: $1" >&2; usage; exit 2;;
  esac
done

if [[ -z "${MODE}" ]]; then
  echo "ERROR: choose an output mode (--print-kmod-version/--print-deb-version/--print-defines/--print-all)" >&2
  usage
  exit 2
fi

if [[ ! -f "${HEADER}" ]]; then
  echo "ERROR: header not found: ${HEADER}" >&2
  exit 1
fi

# Extract STCP_VERSION "x.y.z-..." from header
# Looks for: #define STCP_VERSION "0.0.1-beta"
STCP_VERSION="$(
  awk '
    $1=="#define" && $2=="STCP_VERSION" {
      # join rest, then strip quotes
      v=$3
      gsub(/^"/,"",v); gsub(/"$/,"",v)
      print v
      exit
    }
  ' "${HEADER}"
)"

if [[ -z "${STCP_VERSION}" ]]; then
  echo "ERROR: could not parse STCP_VERSION from ${HEADER}" >&2
  exit 1
fi

# Determine git root
if [[ -z "${GIT_DIR}" ]]; then
  if GIT_DIR="$(git rev-parse --show-toplevel 2>/dev/null)"; then
    :
  else
    GIT_DIR=""
  fi
fi

# Determine SHA
if [[ -n "${OVERRIDE_SHA}" ]]; then
  STCP_GIT_SHA="${OVERRIDE_SHA}"
else
  if [[ -n "${GIT_DIR}" ]] && command -v git >/dev/null 2>&1; then
    STCP_GIT_SHA="$(git -C "${GIT_DIR}" rev-parse --short=12 HEAD 2>/dev/null || true)"
    [[ -n "${STCP_GIT_SHA}" ]] || STCP_GIT_SHA="nogit"
  else
    STCP_GIT_SHA="nogit"
  fi
fi

# Determine build date (UTC, no spaces)
if [[ -n "${OVERRIDE_DATE}" ]]; then
  STCP_BUILD_DATE="${OVERRIDE_DATE}"
else
  STCP_BUILD_DATE="$(date -u +'%Y%m%d-%H%M%SZ')"
fi

# Kmod MODULE_VERSION string (SemVer-friendly)
KMOD_VERSION="${STCP_VERSION}+${STCP_GIT_SHA}.${STCP_BUILD_DATE}"

# Debian version (avoid '-' semantics and keep ordering sane)
# Convert "-beta" -> "~beta" so beta sorts BEFORE final.
# Keep "+git<sha>.<date>" as build metadata-ish.
DEB_BASE="${STCP_VERSION}"
if [[ "${DEB_BASE}" == *"-"* ]]; then
  # only replace first '-' with '~' (0.0.1-beta -> 0.0.1~beta)
  DEB_BASE="${DEB_BASE/-/~}"
fi
DEB_VERSION="${DEB_BASE}+git${STCP_GIT_SHA}.${STCP_BUILD_DATE}"

# Defines for compiler (quotes included)
DEFINES="-DSTCP_GIT_SHA=\"${STCP_GIT_SHA}\" -DSTCP_BUILD_DATE=\"${STCP_BUILD_DATE}\""

case "${MODE}" in
  --print-kmod-version) printf '%s\n' "${KMOD_VERSION}";;
  --print-deb-version)  printf '%s\n' "${DEB_VERSION}";;
  --print-defines)      printf '%s\n' "${DEFINES}";;
  --print-all)
    printf 'STCP_VERSION=%s\n' "${STCP_VERSION}"
    printf 'STCP_GIT_SHA=%s\n' "${STCP_GIT_SHA}"
    printf 'STCP_BUILD_DATE=%s\n' "${STCP_BUILD_DATE}"
    printf 'KMOD_VERSION=%s\n' "${KMOD_VERSION}"
    printf 'DEB_VERSION=%s\n' "${DEB_VERSION}"
    ;;
esac
