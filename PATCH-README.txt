STCPv7 parser progress race fix (14.10.2026)

Files: kernel/module/rust/src/{session,state}.rs

Change: parser_pending atomic bit, unified RX parser ownership for handshake and data, deferred processing after busy guard, Ready wakeup. No wire-format or compression change.

INSTALL (from STCP root, after backup):
  unzip -o stcpv7-parser-race-fix-20261014.zip -d .

VERIFY: compile both Linux kernel module and Zephyr; run STCP-TCP Linux/Linux, Linux/RPi both directions, UDP, AUTO robot tests.

NOT YET COMPILED OR HARDWARE-TESTED in this environment. Review before production.
