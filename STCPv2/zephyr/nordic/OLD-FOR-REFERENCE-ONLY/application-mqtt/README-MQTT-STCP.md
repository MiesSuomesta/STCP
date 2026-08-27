# Zephyr MQTT over STCP

This application is created by duplicating the current known-good
`STCPv2/zephyr/nordic/application` tree into `application-mqtt`.

MQTT protocol handling is Zephyr's own MQTT client library (`CONFIG_MQTT_LIB=y`).
The only STCP-specific part is Zephyr MQTT's custom transport backend.

Transport:
- AF_STCP = 45
- SOCK_STREAM
- IPPROTO_STCP = 253
- MQTT transport type = MQTT_TRANSPORT_CUSTOM

The application implements Zephyr's custom MQTT transport entry points:
- mqtt_client_custom_transport_connect()
- mqtt_client_custom_transport_write()
- mqtt_client_custom_transport_write_msg()
- mqtt_client_custom_transport_read()
- mqtt_client_custom_transport_disconnect()

Configure broker/topic through Kconfig or `mqtt.conf`.

Build:
```sh
bash scripts/build-mqtt.sh
```

Flash:
```sh
bash scripts/flash-mqtt.sh
```
