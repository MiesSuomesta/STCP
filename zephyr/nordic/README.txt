STCP Zephyr pool16 config correction

Extract into /srv/stcp-project/STCP/zephyr/nordic, alongside module-v2 and ethernet.conf.
Run:
  bash fix-pool16-build-flash.sh

Changes only STCP_BUFFER_POOL_FRAMES: module default and the actual nordic/ethernet.conf used by build-ethernet.sh.
Rust session.rs is preserved. Keep SEND_WINDOW=8 and ACK_EVERY_FRAMES=4 on both endpoints.
Build and flash existing Ethernet application; generated .config must contain pool=16 before flash proceeds.
Linux/RPi rebuild is unnecessary if their Rust constants are already 8/4.

Expected log: frame=0/16 (not frame=0/8).
16 shared pool slots provide headroom beyond the 8-frame send window; this addresses the observed pool/window equality, not a proven complete flow-control fix. If exhausted again, capture occupancy and UL/DL/FULL results; do not blindly shrink pool to8.
Existing large slots are unchanged (2x8192 gives 34816 total static buffer bytes with16x1152).
This package requires the previously installed static frame pool module.
