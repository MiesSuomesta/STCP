# Raspberry Pi config.txt activation fix

The generated `install.sh` now:

- writes the new STCP boot block to temporary files on the same filesystem;
- preserves the ownership and mode of the active `config.txt`;
- atomically replaces `config.txt` with `mv`;
- verifies the exact `os_prefix`, `kernel` and `auto_initramfs` settings;
- prints the activated block after installation;
- reports the exact failing line and command if installation stops early;
- removes an old extracted package directory before remote installation;
- records the full remote installation trace in `~/stcp-install-<kernel>.log`;
- verifies the active boot config again after the installer exits.

The kernel, STCP protocol and benchmark sources are otherwise unchanged.
