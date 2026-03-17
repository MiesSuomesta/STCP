# ============================================================
# STCP Zephyr Library
# ============================================================
if(CONFIG_STCP)
	message(STATUS "STCP enabled!")

	if(CONFIG_STCP_DEBUG)
		message(STATUS "STCP debug enabled!")
	else()
		message(STATUS "STCP debug disabled.")
	endif()

	if(CONFIG_STCP_AES_BYPASS)
		message(WARNING "WARNING: STCP AES bypass enabled!")
	else()
		message(STATUS "STCP AES bypass disabled.")
	endif()

	if(CONFIG_MQTT_LIB_STCP)
		message(STATUS "MQTT STCP support enabled.")
	else()
		message(STATUS "MQTT STCP support disabled.")
	endif()

	zephyr_library()

# ------------------------------------------------------------
# C sources (enabled only if CONFIG_STCP)
# ------------------------------------------------------------
	zephyr_library_sources_ifdef(CONFIG_STCP
	          micro-ecc/uECC.c
              src/ffi_random.c
              src/fsm.c
              src/mqtt_transport_socket_stcp.c
              src/network_testing.c
              src/rust_alloc.c
              src/rust_log_with_printk.c
              src/stcp_rx_transmission.c
              src/mqtt_stcp_stats.c
              src/stcp_alloc_zephyr.c
              src/stcp_api.c
              src/stcp_bridge.c
              src/stcp_kernel_crypto.c
              src/stcp_lte_fsm.c
              src/stcp_main_lte.c
              src/stcp_net_zephyr.c
              src/stcp_operations_zephyr.c
              src/stcp_platform_init_lte.c
              src/stcp_socket.c
              src/stcp_soft_reset.c
              src/stcp_tcp_recv.c
              src/stcp_tcp_send.c
              src/stcp_transport_api.c
              src/stcp_transport_lte.c
              src/stcp_wire.c
              src/utils.c
              src/workers.c
              src/lte_workers.c
              src/recovery.c
              src/watchdog.c
              src/debug.c
              src/testing/status-monitor/stat-monitor.c
# TODO:              src/testing/clients/mqtt.c
	)

	zephyr_library_sources_ifdef(CONFIG_STCP_TESTING
              src/testing/the_master_server_thread.c
              src/testing/clients/common.c
              src/testing/clients/raw.c
              src/testing/clients/stcp.c
              src/testing/stcp/stcp_test.c
	)

	set_source_files_properties(
	    src/rust_log_with_printk.c
	    PROPERTIES
	    COMPILE_FLAGS "-Wl,--undefined=stcp_rust_log"
	)

# ------------------------------------------------------------
# Version / metadata
# ------------------------------------------------------------
	set(STCP_VERSION_STR "2.0.0")
	string(TIMESTAMP STCP_BUILD_TIME "%Y-%m-%d %H:%M:%S UTC")

	# Git SHA
	execute_process(
	    COMMAND git describe --tags --dirty --always
	    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
	    OUTPUT_VARIABLE STCP_GIT_SHA
	    OUTPUT_STRIP_TRAILING_WHITESPACE
	    ERROR_QUIET
	)

	if(NOT STCP_GIT_SHA)
	    set(STCP_GIT_SHA "nogit")
	endif()

	zephyr_library_compile_definitions(
	    STCP_VERSION_STR=\"${STCP_VERSION_STR}\"
	    STCP_BUILD_TIME=\"${STCP_BUILD_TIME}\"
	    STCP_GIT_SHA=\"${STCP_GIT_SHA}\"
	)

# ------------------------------------------------------------
# Include paths
# ------------------------------------------------------------
	zephyr_include_directories(
	    ${CMAKE_CURRENT_LIST_DIR}/micro-ecc
	    ${CMAKE_CURRENT_LIST_DIR}/include
	    ${CMAKE_CURRENT_LIST_DIR}/src/testing/include
	    ${CMAKE_CURRENT_LIST_DIR}/src/testing
	    ${CMAKE_CURRENT_LIST_DIR}/../../the-app/stcp-mqtt-demo/include
	    ${CMAKE_CURRENT_BINARY_DIR}
	)

# ------------------------------------------------------------
# Rust build
# ------------------------------------------------------------
	# compile options kaikille lähdetiedostoille tässä moduulissa
	add_compile_definitions(Os flto)

	set(RUST_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/rust)

	set(RUST_LIB_REAL
	    ${RUST_PROJECT_DIR}/target/thumbv8m.main-none-eabi/release/libthe_stcp_kernel_module.a
	)

	zephyr_library_include_directories(
	    include
	    src
	    micro-ecc
	    ${CMAKE_CURRENT_BINARY_DIR}
	)

	add_custom_command(
	    OUTPUT ${RUST_LIB_REAL}

	    COMMAND cargo +nightly build
	            --release --features stcp_debug
        	    --target thumbv8m.main-none-eabi

	    WORKING_DIRECTORY ${RUST_PROJECT_DIR}

	    DEPENDS
        	${RUST_PROJECT_DIR}/Cargo.toml
      		${RUST_PROJECT_DIR}/src/lib.rs
	)

	add_custom_target(stcp_rust_build
	    DEPENDS ${RUST_LIB_REAL}
	)

	add_dependencies(${ZEPHYR_CURRENT_LIBRARY} stcp_rust_build)

	set_source_files_properties(${RUST_LIB_REAL} PROPERTIES GENERATED TRUE)

	zephyr_library_link_libraries(${RUST_LIB_REAL})


# ------------------------------------------------------------
# cbindgen header generation
# ------------------------------------------------------------

	set(STCP_GENERATED_HEADER
	    ${CMAKE_CURRENT_BINARY_DIR}/stcp_generated.h
	)

	zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR})

	add_custom_command(
	    OUTPUT ${STCP_GENERATED_HEADER}

	    COMMAND cbindgen
        	--config ${RUST_PROJECT_DIR}/cbindgen.toml
      		--crate the_stcp_kernel_module 
      		--output ${STCP_GENERATED_HEADER}

	    WORKING_DIRECTORY ${RUST_PROJECT_DIR}

	    DEPENDS
	        ${RUST_PROJECT_DIR}/src/lib.rs
	        ${RUST_PROJECT_DIR}/cbindgen.toml

	    VERBATIM
	)

	set_source_files_properties(${STCP_GENERATED_HEADER} PROPERTIES GENERATED TRUE)
	zephyr_library_sources(${STCP_GENERATED_HEADER})
else()
	message(STATUS "STCP disabled!")
endif()

