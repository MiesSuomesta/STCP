cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stcp_demo)

# ------------------------------------------------------------
# STCP version
# ------------------------------------------------------------
set(STCP_VERSION "1.0.0-beta")

# ------------------------------------------------------------
# Build time (UTC, reproducible)
# ------------------------------------------------------------
string(TIMESTAMP STCP_BUILD_TIME "%Y-%m-%d %H:%M:%S UTC")

# ------------------------------------------------------------
# Git SHA (safe fallback)
# ------------------------------------------------------------
execute_process(
    COMMAND git describe --tags --dirty --always
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE STCP_GIT_SHA
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT STCP_GIT_SHA)
    set(STCP_GIT_SHA "nogit")
endif()

# ------------------------------------------------------------
# Debug
# ------------------------------------------------------------

option(STCP_DEBUG "Enable STCP Rust-debug feature" ON)

set(STCP_RUST_FEATURES "")
if(STCP_DEBUG)
    set(STCP_RUST_FEATURES "stcp_debug")
endif()

# ------------------------------------------------------------
# Inject into Zephyr app target
# ------------------------------------------------------------
target_compile_definitions(app PRIVATE
    STCP_VERSION=\"${STCP_VERSION}\"
    STCP_BUILD_TIME=\"${STCP_BUILD_TIME}\"
    STCP_GIT_SHA=\"${STCP_GIT_SHA}\"
)


#if(CONFIG_STCP_TRANSPORT_LTE)
target_sources(app PRIVATE
	micro-ecc/uECC.c
	src/workers.c
	src/utils.c
  src/network_testing.c
	src/fsm.c
	src/ffi_random.c
	src/rust_alloc.c
	src/rust_log_with_printk.c
	src/stcp_alloc_zephyr.c
	src/stcp_bridge.c
	src/stcp_kernel_crypto.c
	src/stcp_lte_fsm.c
	src/stcp_main_lte.c
	src/stcp_net_zephyr.c
	src/stcp_operations_zephyr.c
	src/stcp_platform_init_lte.c
	src/stcp_soft_reset.c
	src/stcp_tcp_recv.c
	src/stcp_tcp_send.c
	src/stcp_transport_lte.c

    )
#endif()

#if(CONFIG_STCP_TRANSPORT_BT)
#    target_sources(app PRIVATE
#        src/stcp_main_bt.c
#        src/stcp_platform_init_bt.c
#        src/stcp_transport_bt.c
#        src/stcp_platform_bt.c
#    )
#endif()

set(STCP_FOR_LTE 1)
set(STCP_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../out/")

# OOT: anna SDK-polku ulkoa (cmake -DSTCP_SDK_DIR=...)

# --- polut ---
set(RUST_DIR ${CMAKE_CURRENT_LIST_DIR}/rust)
set(RUST_LIB_SRC
  ${RUST_DIR}/target/thumbv8m.main-none-eabi/release/libthe_stcp_kernel_module.a
)

set(STCP_LIB_DST ${STCP_SDK_DIR}/lib/libstcp.a)
set(STCP_INC_DST ${STCP_SDK_DIR}/include)
set(MICRO_INC_DST ${STCP_SDK_DIR}/include)
set(STCP_HDR_DST ${STCP_INC_DST}/stcp.h)

# --- generoi SDK (lib + header) ---
add_custom_command(
  OUTPUT ${STCP_LIB_DST} ${STCP_HDR_DST}

  COMMAND cargo +nightly build
          --release
          --target thumbv8m.main-none-eabi
          --features stcp_debug

  COMMAND ${CMAKE_COMMAND} -E make_directory ${STCP_SDK_DIR}/lib
  COMMAND ${CMAKE_COMMAND} -E make_directory ${STCP_INC_DST}

  # kopioi Rust staticlib
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${RUST_LIB_SRC}
          ${STCP_LIB_DST}

  # aja cbindgen
  COMMAND cbindgen
          --config ${RUST_DIR}/cbindgen.toml
          --crate the_stcp_kernel_module
          --output ${STCP_HDR_DST}

  WORKING_DIRECTORY ${RUST_DIR}

  DEPENDS
    ${RUST_LIB_SRC}
    ${RUST_DIR}/src/lib.rs
    ${RUST_DIR}/cbindgen.toml

  VERBATIM
)

add_custom_target(stcp_sdk ALL
  DEPENDS ${STCP_LIB_DST} ${STCP_HDR_DST}
)

add_dependencies(app stcp_sdk)

# --- include + link ---

target_include_directories(app PRIVATE
  # netti internaalit näkyviin
  ${ZEPHYR_BASE}/subsys/net/ip
  ${ZEPHYR_BASE}/subsys/net/lib/sockets
  # STCP
  ${MICRO_INC_DST}
  ${STCP_INC_DST}
)

target_link_libraries(app PRIVATE
  ${STCP_LIB_DST}
)

target_include_directories(app PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/include
  ${CMAKE_CURRENT_LIST_DIR}/micro-ecc
  ${STCP_SDK_DIR}/include
)

target_link_libraries(app PRIVATE
  ${STCP_SDK_DIR}/lib/libstcp.a
)

