STCP Raspberry Pi package builder v2
====================================

Place the script here:

    RaspberryPI/
    ├── build-rpi-package.sh
    ├── raspberry-kernel-sources/
    └── raspberry-kernel-module/

Pi 5:

    chmod +x build-rpi-package.sh
    ./build-rpi-package.sh pi5

Pi 4:

    ./build-rpi-package.sh pi4

Clean kernel build:

    CLEAN=1 ./build-rpi-package.sh pi5

Output:

    packages/stcp-rpi-*.tar.gz

The script validates:
- ARM64 kernel configuration
- required build tools
- nightly Rust and rust-src
- generated Module.symvers
- AArch64 stcp.ko
- matching kernel release and module vermagic

It creates:
- kernel Image
- all kernel modules
- DTBs and overlays
- stcp.ko
- depmod metadata
- manifest.txt
- safe Raspberry Pi installer
