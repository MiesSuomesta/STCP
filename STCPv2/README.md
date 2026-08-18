# STCPv2 source tree

This tree is a structural refactor of the supplied STCPv2 support bundle.
The STCP SDK is intentionally not part of this repository.

## Layout

- `kernel/module/` — one Linux kernel module source tree shared by x86_64 host and Raspberry Pi (`PLATFORM=host|rpi`).
- `zephyr/nordic/module/` — Zephyr STCP module.
- `zephyr/nordic/application/` — nRF9151 STCP test/benchmark application.
- `tests/kernel/` — kernel-module integration, stress and Python tests.
- `tests/benchmark/` — platform benchmark tooling/results (Linux, Raspberry Pi, Zephyr as available in the bundle).
- `tests/robot/` — Robot Framework test results/artifacts from the support bundle.
- `tests/zephyr/echo-server/` — external Zephyr echo-server test helper.
- `artifacts/` — build/runtime logs and support-bundle metadata; not source code.
- `docs/` — repository-level documentation.

## Important design choice

The Linux/Raspberry Pi kernel implementation was not split by platform because the supplied Makefile explicitly uses one source tree for both platforms. Keeping it as `kernel/module/` reflects the actual implementation and avoids duplicate platform trees.

Zephyr build scripts now derive repository-relative paths instead of hard-coding `/home/pomo/git/STCP/STCPv2/...`.

## Build and install entrypoints

Build all targets available on the current build machine:

```bash
./scripts/build-all.sh
```

Build selected targets:

```bash
./scripts/build-all.sh host
RPI_KDIR=/path/to/rpi/kernel ./scripts/build-all.sh rpi
NCS_DIR=/path/to/ncs ./scripts/build-all.sh zephyr
```

Kernel artifacts are staged separately under `artifacts/build/kernel-host/` and
`artifacts/build/kernel-rpi/`, so building one target cannot overwrite the
installable artifact of the other.

Install the staged kernel module matching the current machine:

```bash
./scripts/install-all.sh host
```

Deploy Raspberry Pi or flash a Zephyr build explicitly:

```bash
# Deploy the staged Raspberry Pi module over SSH (defaults: host=raspi, user=$USER)
./scripts/install-all.sh rpi

# Override the Raspberry Pi SSH target when needed
RPI_HOST=192.168.1.50 RPI_USER=pomo ./scripts/install-all.sh rpi

NCS_DIR=/path/to/ncs ./scripts/install-all.sh zephyr-nrf9151
NCS_DIR=/path/to/ncs ./scripts/install-all.sh zephyr-ethernet
```

`install-all.sh kernel` remains an alias for `host`. `install-all.sh rpi` deploys `artifacts/build/kernel-rpi/stcp.ko` to the Raspberry Pi over SSH, checks the remote kernel/vermagic, runs `depmod`, reloads STCP and verifies the installed module checksum.

`install-all.sh all` installs the local host kernel module and flashes one Zephyr
variant. Select it with `ZEPHYR_VARIANT=nrf9151` or `ethernet`; the script never
flashes both variants consecutively because the latter would just replace the
former on the board.

## Raspberry Pi kernel + STCP build/deploy

The `rpi` target is an atomic kernel build: it builds the Raspberry Pi `Image`, in-tree modules and DTBs first, then builds the external STCP module against that exact kernel and stages the complete module tree.

```bash
./scripts/build-all.sh rpi
./scripts/install-all.sh rpi
```

Default Raspberry kernel tree: `kernel/raspberry`. Override with `RPI_KDIR=/path/to/tree`. The installer deploys the versioned kernel image and complete `/lib/modules/<kernel-release>` tree over SSH. It does not reboot by default; use `RPI_REBOOT=1 ./scripts/install-all.sh rpi` to reboot after a successful install.
