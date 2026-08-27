STCP Zephyr app-mqtt Kconfig symbols fix

Copy Kconfig to the app-mqtt application root.

Defines only the application-level symbols used by src/main.c:
  CONFIG_STCP_MQTT_BROKER_IPV4
  CONFIG_STCP_MQTT_BROKER_PORT
  CONFIG_STCP_MQTT_TOPIC
  CONFIG_STCP_MQTT_CLIENT_ID
  CONFIG_STCP_MQTT_PAYLOAD
  CONFIG_STCP_MQTT_PUBLISH_INTERVAL_MS
  CONFIG_STCP_MQTT_RECONNECT_DELAY_MS

The normal Zephyr Kconfig tree is still included with:
  source "Kconfig.zephyr"

No module-v2, kernel, shared Rust, or protocol changes.
