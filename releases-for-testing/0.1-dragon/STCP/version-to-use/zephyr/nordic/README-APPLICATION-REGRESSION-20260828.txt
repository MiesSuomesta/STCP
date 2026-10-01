STCP Nordic application-protocol regression overlay
======================================================

Based on nordic-28082026_145147.zip.
No kernel/module-v2/shared Rust files are changed.

Adds independent application Robot suites:

  app-coap/testing/robot-app/coap.robot
    - STCP-UDP/254 connect + gateway accept
    - GET reaches native UDP backend
    - 2.05 Content response reaches Zephyr
    - at least 3 repeated round trips
    - self-contained Python CoAP test backend

  app-mqtt/testing/robot-app/mqtt.robot
    - STCP-TCP/253 gateway accept
    - MQTT CONNECT / CONNACK
    - Zephyr -> Linux QoS0 PUBLISH
    - at least 3 repeated publishes
    - self-contained minimal MQTT 3.1.1 test broker

Run sequence
------------
1. Build + flash CoAP application:
   app-coap/scripts/build-v2-clean.sh
   app-coap/scripts/flash-v2-clean.sh
   app-coap/scripts/run-application-robot.sh

2. Build + flash MQTT application:
   app-mqtt/scripts/build-v2-clean.sh
   app-mqtt/scripts/flash-v2-clean.sh
   app-mqtt/scripts/run-application-robot.sh

The Robot runner builds its Rust STCP gateway itself.
The backend does NOT need libcoap or mosquitto; the test backends are included.

Config-only changes:
- CoAP request interval = 2000 ms for regression speed.
- MQTT publish interval = 2000 ms for regression speed.
- MQTT W5500 RX thread stack = 16384, matching the working CoAP setting and
  the observed 8192-byte eth_w5500 stack overflow.

This intentionally does not integrate into the golden FULL runner yet. First
prove both application suites independently; then add them as post-golden
stages so an app failure never obscures transport regression results.
