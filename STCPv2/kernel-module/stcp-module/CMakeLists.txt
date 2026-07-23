message(STATUS "Loading STCP2 offload module from ${CMAKE_CURRENT_LIST_DIR}")
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
zephyr_library_named(stcp)
zephyr_library_sources_ifdef(CONFIG_STCP
    src/stcp_context.c
    src/stcp_carrier.c
    src/stcp_socket_offload.c
)
