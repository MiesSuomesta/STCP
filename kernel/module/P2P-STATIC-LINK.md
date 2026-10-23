# P2P static linking on Linux and Raspberry Pi

The P2P core is already compiled into `libstcp_kernel_core.a` through
`rust/src/lib.rs` (`mod p2p`). The module Makefile extracts that archive and
prelinks its object files into `src/rust_core.o`; Kbuild then includes it in
`stcp.ko`. Host x86_64 and Raspberry Pi AArch64 use this same source tree.
No separate P2P shared library or kernel module is needed.

The build now verifies the target architecture and all 16 P2P Rust ABI function
definitions in `rust_core.o`. After Kbuild it verifies those definitions and
the two C Noise crypto bridge definitions in the final `stcp.ko` (18 total).
Undefined references and local/private symbols do not satisfy this check.
A failed check fails the build. `NM_TOOL`, `READELF_TOOL` and `PYTHON` may be
overridden to select the build tools.

Build through the normal project entry points:

```bash
cd /srv/stcp-project
bash scripts/build.sh --host
bash scripts/build.sh --rpi
```

Or use the module Makefile directly with the appropriate kernel tree/toolchain:

```bash
make PLATFORM=host KDIR=/path/to/host/kernel
make PLATFORM=rpi KDIR=/path/to/rpi/kernel CROSS_COMPILE=aarch64-linux-gnu-
```

Expected build output:

```text
[OK] Static P2P link: .../rust_core.o (x86_64, 16 definitions)
[OK] Static P2P link: .../stcp.ko (x86_64, 18 definitions)
```

The Raspberry Pi build prints `arm64` instead of `x86_64`.

On module load, the existing init code runs the P2P selftest, including a
complete Noise XX initiator/responder exchange and directional split-key
comparison. Check the target machine after installing/loading its new module:

```bash
sudo dmesg | rg 'stcp:.*P2P'
```

Expected existing message:

```text
stcp: P2P shared core ABI=2 Noise XX initiator/responder selftest passed
```

## Capability boundary

Static link presence is not full libp2p network interoperability. The existing
core provides multistream framing and Noise XX identity/handshake machinery.
`stcp_p2p_noise_core_ready()` is 1, but `stcp_p2p_core_ready()` remains 0 because
the complete Yamux and ping stream state machines are not implemented. This
change does not add a new P2P socket protocol, wire mode, NAT discovery or a
benchmark backend. The Zephyr P2P benchmark's weak backend defaults do not
become functional merely because the core is linked.

The current protocol design places Noise above the existing STCP-TCP byte
stream. STCP AES-GCM remains the transport encryption; Noise's ChaChaPoly is
its own layer, not a substitute for STCP crypto.

## Host validation

```bash
python3 testing/p2p/run.py
```

Requires Cargo/Rust 2024 support, a C compiler and OpenSSL development headers.
Uses the actual production P2P, Noise and KDF source with OpenSSL test backends,
not the kernel crypto implementation. It verifies both C ABI roles, rejects a
malformed handshake, runs the core selftest, and asserts the honest readiness
boundary. Host tests do not claim live kernel, hardware or libp2p interoperability.

For an installed build artifact, the link check can also be run directly:

```bash
python3 scripts/check-p2p-link.py --arch x86_64 stcp.ko
python3 scripts/check-p2p-link.py --arch arm64 --nm aarch64-linux-gnu-nm stcp.ko
```
