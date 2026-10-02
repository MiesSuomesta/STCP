STCP Zephyr config overlay

Copy nordic/ over STCP/version-to-use/zephyr/nordic/.

Layers:
  nordic/common.conf       common Zephyr/STCP settings
  nordic/lte.conf          LTE transport only
  nordic/ethernet.conf     W5500/Ethernet transport only
  application/prj.conf     benchmark/MQTT application only

Build scripts use prj.conf automatically and add:
  common.conf + lte.conf
or
  common.conf + ethernet.conf

This overlay intentionally does not delete the old per-application ethernet.conf/nrf9151.conf files; they become unused by these common build scripts and can be removed after successful tests.
