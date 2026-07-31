# W5500 MAC and overlay correction

- Fixed the application overlay to reference the actual shield node label: `&eth_w5500`.
- Added locally administered MAC address `02:00:00:00:91:51`.
- Limited W5500 SPI frequency to 4 MHz for TX-path bring-up.
- Removed the unsupported `CONFIG_ETH_W5500_LOG_LEVEL_DBG` symbol.
- Ethernet build now uses `CONF_FILE=ethernet.conf`, preventing LTE Kconfig settings from leaking into the Ethernet firmware.
- Static IPv4 configuration remains `192.168.1.50/24`, gateway `192.168.1.111`.
