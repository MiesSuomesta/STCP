cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stcp_mqtt)

# Reject mixed carriers before building firmware.
if(CONFIG_STCP_TRANSPORT_ETHERNET)
    if(NOT CONFIG_ETH_W5500 OR CONFIG_NRF_MODEM_LIB)
        message(FATAL_ERROR "Ethernet transport requires W5500 and modem disabled")
    endif()
else()
    if(CONFIG_ETH_W5500 OR CONFIG_NET_L2_ETHERNET OR CONFIG_NET_CONFIG_AUTO_INIT)
        message(FATAL_ERROR "LTE transport must disable W5500, Ethernet L2 and automatic LAN configuration")
    endif()
    if(NOT CONFIG_NRF_MODEM_LIB)
        message(FATAL_ERROR "LTE transport requires CONFIG_NRF_MODEM_LIB=y")
    endif()
endif()

# Fail before compilation if the Ethernet pool fragment was omitted/overridden.
if(CONFIG_ETH_W5500)
    if(NOT "${CONFIG_NET_BUF_DATA_SIZE}" STREQUAL "1536")
        message(FATAL_ERROR "Ethernet RX pool configuration mismatch: CONFIG_NET_BUF_DATA_SIZE=${CONFIG_NET_BUF_DATA_SIZE}; expected 1536. Include application's ethernet.conf after other configuration fragments and run pristine build.")
    endif()
    if(NOT "${CONFIG_NET_BUF_RX_COUNT}" STREQUAL "10")
        message(FATAL_ERROR "Ethernet RX pool configuration mismatch: CONFIG_NET_BUF_RX_COUNT=${CONFIG_NET_BUF_RX_COUNT}; expected 10. Include application's ethernet.conf after other configuration fragments and run pristine build.")
    endif()
    if(NOT "${CONFIG_NET_BUF_TX_COUNT}" STREQUAL "6")
        message(FATAL_ERROR "Ethernet RX pool configuration mismatch: CONFIG_NET_BUF_TX_COUNT=${CONFIG_NET_BUF_TX_COUNT}; expected 6. Include application's ethernet.conf after other configuration fragments and run pristine build.")
    endif()
    if(NOT "${CONFIG_NET_PKT_RX_COUNT}" STREQUAL "16")
        message(FATAL_ERROR "Ethernet RX pool configuration mismatch: CONFIG_NET_PKT_RX_COUNT=${CONFIG_NET_PKT_RX_COUNT}; expected 16. Include application's ethernet.conf after other configuration fragments and run pristine build.")
    endif()
    if(NOT "${CONFIG_NET_PKT_TX_COUNT}" STREQUAL "8")
        message(FATAL_ERROR "Ethernet RX pool configuration mismatch: CONFIG_NET_PKT_TX_COUNT=${CONFIG_NET_PKT_TX_COUNT}; expected 8. Include application's ethernet.conf after other configuration fragments and run pristine build.")
    endif()
    message(STATUS "W5500 net pools: data=1536 RX=10 TX=6 packets RX=16 TX=8")
endif()


target_sources(app PRIVATE
    src/main.c
    src/echo_benchmark.c
    src/mqtt_codec.c
    src/mqtt_proxy.c
)

target_sources_ifdef(CONFIG_STCP_BENCH_SHELL app PRIVATE
    src/bench_shell.c
)

if(CONFIG_STCP_BENCH_SHELL AND CONFIG_NET_NATIVE)
    target_sources(app PRIVATE
        src/stcp_ping.c
    )
endif()

target_sources_ifdef(CONFIG_NRF_MODEM_LIB app PRIVATE
    src/stcp_lte_transport.c
)

if(CONFIG_STCP_BENCH_SHELL AND CONFIG_NRF_MODEM_LIB)
    target_sources(app PRIVATE src/modem_status.c)
endif()

target_sources_ifdef(CONFIG_ETH_W5500 app PRIVATE
    src/ethernet_status.c
)
