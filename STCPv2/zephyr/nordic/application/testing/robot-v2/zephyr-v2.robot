*** Settings ***
Library          libraries/ZephyrSerial.py
Library          Process
Suite Setup      Start STCPv2 Matrix
Suite Teardown   Stop STCPv2 Matrix

*** Variables ***
${SERIAL}        %{STCP_ZEPHYR_SERIAL=/dev/ttyACM0}
${BAUD}          %{STCP_ZEPHYR_BAUD=115200}
${SERVER_HOST}   %{STCP_ZEPHYR_SERVER_HOST=192.168.1.20}
${SERVER_PORT}   %{STCP_ZEPHYR_SERVER_PORT=13000}
${TOTAL}         %{STCP_ZEPHYR_TOTAL=262144}
${CHUNK}         %{STCP_ZEPHYR_CHUNK=4096}
${SERVER_BIN}    %{STCP_ZEPHYR_SERVER_BIN=./server/stcp-v2-bench-server}

*** Keywords ***
Start STCPv2 Matrix
    Start Process    ${SERVER_BIN}    0.0.0.0    ${SERVER_PORT}    alias=stcp_server    stdout=server.stdout.log    stderr=server.stderr.log
    Sleep    1s
    Open Zephyr Serial    ${SERIAL}    ${BAUD}
    Run Shell Command    stcp config host ${SERVER_HOST}
    Run Shell Command    stcp config port ${SERVER_PORT}
    Run Shell Command    stcp config total ${TOTAL}
    Run Shell Command    stcp config chunk ${CHUNK}

Stop STCPv2 Matrix
    Close Zephyr Serial
    Terminate Process    stcp_server    kill=True

Use Transport
    [Arguments]    ${transport}
    ${out}=    Run Shell Command    stcp config transport ${transport}
    Should Contain    ${out}    Transport = ${transport}

*** Test Cases ***
Native Network Is Alive
    ${out}=    Run Shell Command    net ping ${SERVER_HOST}    timeout=8
    Should Contain    ${out}    bytes from

STCPv2 Download From Linux
    Use Transport    stcp
    ${r}=    Run Benchmark    stcp bench download    timeout=30
    Should Be Equal As Integers    ${r}[status]    0

STCPv2 Upload To Linux
    Use Transport    stcp
    ${r}=    Run Benchmark    stcp bench upload    timeout=30
    Should Be Equal As Integers    ${r}[status]    0

STCPv2 Full Duplex With Linux
    Use Transport    stcp
    ${r}=    Run Benchmark    stcp bench full    timeout=30
    Should Be Equal As Integers    ${r}[status]    0
