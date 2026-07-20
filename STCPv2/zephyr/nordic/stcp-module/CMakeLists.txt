message(STATUS "Loading STCP OOT module from ${CMAKE_CURRENT_LIST_DIR}")

zephyr_include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/include
)

zephyr_library_named(stcp)

zephyr_library_sources(
    src/stcp_context.c
    src/stcp_core_stub.c
    src/stcp_socket_offload.c
)

