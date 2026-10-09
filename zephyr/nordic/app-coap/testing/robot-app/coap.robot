*** Settings ***
Library          libraries/ZephyrSerial.py
Library          Process
Suite Setup      Start CoAP E2E
Suite Teardown   Stop CoAP E2E

*** Variables ***
${SERIAL}          %{STCP_ZEPHYR_SERIAL=/dev/ttyACM0}
${BAUD}            %{STCP_ZEPHYR_BAUD=115200}
${GATEWAY_BIN}     %{STCP_COAP_GATEWAY_BIN=../gw/target/release/stcp-v2-coap-gateway}
${GATEWAY_HOST}    %{STCP_COAP_GATEWAY_HOST=0.0.0.0}
${GATEWAY_PORT}    %{STCP_COAP_GATEWAY_PORT=56830}
${BACKEND_HOST}    %{STCP_COAP_BACKEND_HOST=127.0.0.1}
${BACKEND_PORT}    %{STCP_COAP_BACKEND_PORT=5683}

*** Keywords ***
Start CoAP E2E
    # Open serial first. The firmware is already running after flash and the
    # first successful STCP handshake marker is one-shot; opening serial after
    # the gateway can miss it completely.
    Open Zephyr Serial    ${SERIAL}    ${BAUD}
    Sleep    300ms
    Start Process    python3    backends/coap_backend.py    ${BACKEND_HOST}    ${BACKEND_PORT}    alias=coap_backend    stdout=coap-backend.log    stderr=STDOUT
    Sleep    300ms
    Start Process    ${GATEWAY_BIN}    ${GATEWAY_HOST}    ${GATEWAY_PORT}    ${BACKEND_HOST}:${BACKEND_PORT}    alias=coap_gateway    stdout=coap-gateway.log    stderr=STDOUT

Stop CoAP E2E
    Close Zephyr Serial
    Terminate Process    coap_gateway    kill=True
    Terminate Process    coap_backend    kill=True

*** Test Cases ***
CoAP STCP UDP Handshake And Gateway Accept
    ${s}=    Wait For Serial Text    CoAP STCP-UDP transport connected    timeout=90
    Should Contain    ${s}    transport connected
    ${g}=    Wait For File Text    coap-gateway.log    STCP-UDP client accepted    timeout=90
    Should Contain    ${g}    STCP-UDP client accepted

CoAP GET Reaches Native UDP Backend
    ${b}=    Wait For File Text    coap-backend.log    COAP_TEST_REQUEST    timeout=15
    Should Contain    ${b}    COAP_TEST_REQUEST

CoAP Response Returns To Zephyr
    ${s}=    Wait For Serial Text    CoAP response code=2.05    timeout=15
    Should Contain    ${s}    CoAP response code=2.05
    ${s2}=    Wait For Serial Text    CoAP payload: STCP_COAP_OK    timeout=15
    Should Contain    ${s2}    STCP_COAP_OK

CoAP Repeated Round Trips
    Sleep    7s
    ${count}=    Count File Text    coap-backend.log    COAP_TEST_RESPONSE
    Should Be True    ${count} >= 3    Expected at least 3 CoAP responses, got ${count}
