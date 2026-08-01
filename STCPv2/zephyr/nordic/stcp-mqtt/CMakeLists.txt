cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stcp_mqtt)

target_sources(app PRIVATE
    src/main.c
    src/echo_benchmark.c
    src/mqtt_codec.c
    src/mqtt_proxy.c
    src/stcp_ping.c
)

target_sources_ifdef(CONFIG_STCP_BENCH_SHELL app PRIVATE
    src/bench_shell.c
)

target_sources_ifdef(CONFIG_NRF_MODEM_LIB app PRIVATE
    src/stcp_lte_transport.c
)

if(CONFIG_STCP_BENCH_SHELL AND CONFIG_NRF_MODEM_LIB)
    target_sources(app PRIVATE src/modem_status.c)
endif()

target_sources_ifdef(CONFIG_ETH_W5500 app PRIVATE
    src/ethernet_status.c
)
