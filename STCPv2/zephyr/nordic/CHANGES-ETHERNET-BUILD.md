# Ethernet build separation

- Added `ethernet.conf` for native Zephyr IPv4, DHCPv4 and Seeed W5500.
- Added `scripts/build-ethernet.sh`.
- LTE transport source is compiled only when `CONFIG_NRF_MODEM_LIB=y`.
- Modem status and AT shell are excluded from Ethernet firmware.
- LTE initialization and SO_BINDTOPDN calls are compiled only in LTE builds.
- Ethernet benchmark summary reports the native Ethernet backend instead of modem metrics.
