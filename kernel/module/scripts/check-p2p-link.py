#!/usr/bin/env python3
"""Reject Linux/RPi build artifacts missing the statically linked P2P core."""
import os
import argparse
import subprocess
import sys
from pathlib import Path

REQUIRED = (
    'stcp_p2p_core_abi_version', 'stcp_p2p_core_selftest',
    'stcp_p2p_core_ready', 'stcp_p2p_core_stage_name',
    'stcp_p2p_noise_core_ready', 'stcp_p2p_noise_selftest',
    'stcp_p2p_noise_dialer_new', 'stcp_p2p_noise_dialer_free',
    'stcp_p2p_noise_dialer_message1', 'stcp_p2p_noise_dialer_message2',
    'stcp_p2p_noise_dialer_complete',
    'stcp_p2p_noise_listener_new', 'stcp_p2p_noise_listener_free',
    'stcp_p2p_noise_listener_message1', 'stcp_p2p_noise_listener_message3',
    'stcp_p2p_noise_listener_complete',
    'stcp_p2p_noise_chacha_encrypt', 'stcp_p2p_noise_chacha_decrypt',
)
# The crypto bridge is C, so only the Rust ABI is expected in rust_core.o.
RUST_REQUIRED = REQUIRED[:-2]

def run(tool, *args):
    env = os.environ.copy()
    env["LC_ALL"] = "C"
    p = subprocess.run([tool, *args], text=True, capture_output=True, env=env)
    if p.returncode:
        raise ValueError(f'{tool} failed: {p.stderr.strip()}')
    return p.stdout

def check(path, nm, readelf, arch, rust_only):
    if not path.is_file():
        raise ValueError(f'missing artifact: {path}')
    header = run(readelf, '-h', str(path))
    machine = next((line.split(':', 1)[1].strip() for line in header.splitlines()
                    if line.strip().startswith('Machine:')), '')
    expected = 'AArch64' if arch == 'arm64' else 'Advanced Micro Devices X86-64'
    if machine != expected:
        raise ValueError(f'{path}: expected {expected}, found {machine!r}')
    symbols = set()
    for line in run(nm, '--defined-only', '--format=posix', str(path)).splitlines():
        fields = line.split()
        if len(fields) >= 2 and fields[1] == 'T':
            symbols.add(fields[0])
    required = RUST_REQUIRED if rust_only else REQUIRED
    missing = [name for name in required if name not in symbols]
    if missing:
        raise ValueError(f'{path}: missing static P2P definitions: {", ".join(missing)}')
    print(f'[OK] Static P2P link: {path} ({arch}, {len(required)} definitions)')

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--nm', default='nm')
    ap.add_argument('--readelf', default='readelf')
    ap.add_argument('--arch', choices=('x86_64', 'arm64'), required=True)
    ap.add_argument('--rust-only', action='store_true')
    ap.add_argument('artifacts', nargs='+', type=Path)
    args = ap.parse_args()
    try:
        for path in args.artifacts:
            check(path, args.nm, args.readelf, args.arch, args.rust_only)
    except (OSError, ValueError) as exc:
        print(f'[FAIL] {exc}', file=sys.stderr)
        return 1
    return 0

if __name__ == '__main__':
    sys.exit(main())
