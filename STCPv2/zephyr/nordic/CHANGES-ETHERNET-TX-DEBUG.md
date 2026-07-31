# Ethernet TX diagnostics

- Uses static IPv4 `192.168.1.50/24`.
- Correct gateway for the shown LAN is `192.168.1.111`.
- DHCP is disabled.
- W5500 SPI clock is limited to 4 MHz during bring-up.
- W5500 driver debug and SPI informational logs are enabled.
- Adds `stcp net status` to show interface, carrier, MAC, IPv4 and gateway.
- Logs the same Ethernet status shortly after boot.

Build with `stcp-mqtt/scripts/build-ethernet.sh`.
