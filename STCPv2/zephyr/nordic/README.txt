STCPv2 Zephyr MQTT project creator

1. Extract this ZIP from the STCP repository root.
2. Run:
     bash STCPv2/zephyr/nordic/create-mqtt-application.sh
3. It copies the CURRENT golden application -> application-mqtt.
4. It replaces only application-level sources/config with the MQTT project.
5. module-v2 and shared Rust core remain external and untouched.
