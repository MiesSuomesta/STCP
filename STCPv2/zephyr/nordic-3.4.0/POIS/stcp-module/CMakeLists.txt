message(STATUS "Loading STCP Zephyr C port from ${CMAKE_CURRENT_LIST_DIR}")

zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

# The STCP module lives directly under the west workspace root in this project:
#   <west-topdir>/stcp-module
#   <west-topdir>/nrfxlib
#
# NCS 3.4 can enable nrf/lib/at_monitor before exporting the nRF Modem Library
# public include path to an OOT module build. Add the public header directory
# explicitly so nrf_modem_at.h is available to at_monitor and STCP LTE code.
get_filename_component(STCP_WEST_TOPDIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(STCP_NRF_MODEM_INCLUDE "${STCP_WEST_TOPDIR}/nrfxlib/nrf_modem/include")

if(CONFIG_NRF_MODEM_LIB)
    if(EXISTS "${STCP_NRF_MODEM_INCLUDE}/nrf_modem_at.h")
        message(STATUS "STCP: adding nRF Modem include path: ${STCP_NRF_MODEM_INCLUDE}")
        zephyr_include_directories("${STCP_NRF_MODEM_INCLUDE}")
    else()
        message(FATAL_ERROR
            "CONFIG_NRF_MODEM_LIB=y, but nrf_modem_at.h was not found at "
            "${STCP_NRF_MODEM_INCLUDE}. Run 'west update nrfxlib'.")
    endif()
endif()

zephyr_library_named(stcp)
zephyr_library_sources_ifdef(CONFIG_STCP
    src/stcp_context.c
    src/stcp_carrier.c
    src/stcp_core.c
    src/stcp_socket_offload.c
)

zephyr_library_sources_ifdef(CONFIG_STCP_LTE
    src/stcp_lte.c
)
