STCPv5 Zephyr all-app config overlay

Layout:
  nordic/common.conf                 shared Zephyr/STCP base
  nordic/lte.conf                    LTE transport only
  nordic/ethernet.conf               W5500 transport only
  nordic/<app>/prj.conf              application-only settings
  nordic/<app>/boards/*_w5500.overlay Ethernet-only devicetree overlay
  nordic/common-scripts/build-*.sh   shared builders, optional app name argument

Examples:
  common-scripts/build-ethernet.sh application
  common-scripts/build-ethernet.sh app-coap
  common-scripts/build-ethernet.sh app-mqtt
  common-scripts/build-ethernet.sh p2p-application

  common-scripts/build-lte.sh application
  common-scripts/build-lte.sh app-coap
  common-scripts/build-lte.sh app-mqtt
  common-scripts/build-lte.sh p2p-application

Old per-app ethernet.conf / stcp-v2-clean.conf files are intentionally not deleted by this overlay.
Remove them only after all builds/tests are green.
