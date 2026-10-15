*** Settings ***
Documentation    Real Linux STCP-TCP server/client tests. Requires loaded STCP module.
Library          StcpLibrary.py
Suite Setup      Prepare SDK    golang    ${PORT}    ${OUTPUT DIR}
Suite Teardown   Stop Echo Server
Test Setup       Start Echo Server
Test Teardown    Stop Echo Server
Test Timeout     45 seconds
Force Tags       stcp    golang

*** Variables ***
${PORT}          19703

*** Test Cases ***
ASCII Echo
    Client Echo Should Pass    Hello STCP!

Unicode Echo
    Client Echo Should Pass    Terve ääkköset 日本 😀

Long Message Echo
    Long Client Message Should Pass

Binary Server Echo 256 KiB
    Binary Echo Should Pass

Ten Sequential Connections
    Repeated Connections Should Pass

Server Survives Empty Disconnect
    Server Should Survive Disconnect

Invalid Arguments Are Rejected
    Invalid Arguments Should Fail

Client Rejects Corrupted Echo
    Client Should Detect Bad Echo    wrong

Client Rejects Premature EOF
    Client Should Detect Bad Echo    close

Client Handles Fragmented Echo
    Client Should Detect Bad Echo    fragment
