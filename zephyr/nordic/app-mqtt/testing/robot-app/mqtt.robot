*** Settings ***
Library          libraries/ZephyrSerial.py
Library          Process
Suite Setup      Start MQTT E2E
Suite Teardown   Stop MQTT E2E

*** Variables ***
${SERIAL}          %{STCP_ZEPHYR_SERIAL=/dev/ttyACM0}
${BAUD}            %{STCP_ZEPHYR_BAUD=115200}
${GATEWAY_BIN}     %{STCP_MQTT_GATEWAY_BIN=../gw/target/release/stcp-v2-mqtt-gateway}
${GATEWAY_HOST}    %{STCP_MQTT_GATEWAY_HOST=0.0.0.0}
${GATEWAY_PORT}    %{STCP_MQTT_GATEWAY_PORT=18830}
${BACKEND_HOST}    %{STCP_MQTT_BACKEND_HOST=127.0.0.1}
${BACKEND_PORT}    %{STCP_MQTT_BACKEND_PORT=1884}

*** Keywords ***
Start MQTT E2E
    # Same startup rule as CoAP: capture serial before making the gateway
    # available so the one-shot connect/CONNACK markers cannot race past Robot.
    Open Zephyr Serial    ${SERIAL}    ${BAUD}
    Sleep    300ms
    Start Process    python3    backends/mqtt_broker.py    ${BACKEND_HOST}    ${BACKEND_PORT}    alias=mqtt_backend    stdout=mqtt-backend.log    stderr=STDOUT
    Sleep    300ms
    Start Process    ${GATEWAY_BIN}    ${GATEWAY_HOST}    ${GATEWAY_PORT}    ${BACKEND_HOST}    ${BACKEND_PORT}    alias=mqtt_gateway    stdout=mqtt-gateway.log    stderr=STDOUT

Stop MQTT E2E
    Close Zephyr Serial
    Terminate Process    mqtt_gateway    kill=True
    Terminate Process    mqtt_backend    kill=True

*** Test Cases ***
MQTT STCP TCP Gateway Accepts Client
    ${g}=    Wait For File Text    mqtt-gateway.log    STCP client accepted    timeout=90
    Should Contain    ${g}    STCP client accepted
    ${g2}=    Wait For File Text    mqtt-gateway.log    MQTT backend connected    timeout=30
    Should Contain    ${g2}    MQTT backend connected

MQTT CONNECT CONNACK Over STCP
    ${b}=    Wait For File Text    mqtt-backend.log    MQTT_TEST_CONNACK_OK    timeout=30
    Should Contain    ${b}    MQTT_TEST_CONNACK_OK
    ${s}=    Wait For Serial Text    MQTT CONNACK OK    timeout=30
    Should Contain    ${s}    MQTT CONNACK OK

MQTT Publish Zephyr To Linux Broker
    ${b}=    Wait For File Text    mqtt-backend.log    MQTT_TEST_PUBLISH topic=stcp/demo    timeout=15
    Should Contain    ${b}    hello from Zephyr over STCP

MQTT Repeated Publishes
    Sleep    7s
    ${serial}=    Dump Serial    15.0
    Log    ${serial}
    ${count}=    Count File Text    mqtt-backend.log    MQTT_TEST_PUBLISH
    Should Be True    ${count} >= 3    Expected at least 3 MQTT publishes, got ${count}
