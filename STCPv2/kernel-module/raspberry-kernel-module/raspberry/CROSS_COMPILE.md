# Cross compile x86_64 host -> Raspberry Pi ARM64

The kernel module must be linked against the exact Raspberry Pi kernel build tree used on the target.

## Host packages (Debian)

```bash
sudo apt install \
  git bc bison flex libssl-dev libelf-dev dwarves \
  gcc-aarch64-linux-gnu binutils-aarch64-linux-gnu \
  clang llvm lld rustup

rustup toolchain install nightly --component rust-src
```

## Prepare Raspberry Pi kernel tree

Clone the same kernel branch/commit as the target Raspberry Pi kernel. Then configure it.

Raspberry Pi 4:

```bash
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig
```

Raspberry Pi 5:

```bash
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2712_defconfig
```

Prepare external-module headers:

```bash
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- olddefconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -j$(nproc) modules_prepare
```

If the target kernel has `CONFIG_MODVERSIONS=y`, copy the matching `Module.symvers` from the complete target kernel build, or build the complete kernel once. `modules_prepare` alone does not generate `Module.symvers`.

## Build STCP

```bash
KDIR=$HOME/src/raspberrypi-linux \
  ./raspberry/build-cross-x86.sh
```

The result is `stcp.ko`. Copy it to the Raspberry Pi and verify before loading:

```bash
scp stcp.ko pi@raspberry:/tmp/
ssh pi@raspberry 'uname -r; modinfo /tmp/stcp.ko | grep vermagic'
```

The module's `vermagic` must match the target kernel. Run the smoke test on the Raspberry Pi, not on the x86 host.
