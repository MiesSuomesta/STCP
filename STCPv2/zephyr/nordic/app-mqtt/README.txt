STCP Zephyr app-mqtt Kconfig choice-symbol fix.

Base:
- user's known-good Kconfig preserved
- MQTT app symbols appended

Fix:
- removed `select MQTT_VERSION_3_1_1`
- MQTT_VERSION_3_1_1 is a Kconfig choice symbol and select/imply is invalid
- MQTT library's own choice defaults to MQTT_VERSION_3_1_1

Still selects:
- MQTT_LIB
- MQTT_LIB_CUSTOM_TRANSPORT
