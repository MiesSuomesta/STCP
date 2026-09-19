message(STATUS "Loading clean STCPv2 Zephyr adapter from ${CMAKE_CURRENT_LIST_DIR}")

zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
zephyr_library_named(stcp_v2)
zephyr_library_sources_ifdef(CONFIG_STCP_V2
    src/stcp_v2_context.c
    src/stcp_v2_carrier.c
    src/stcp_v2_rx.c
    src/stcp_v2_socket.c
    src/stcp_v2_platform.c
    src/stcp_x25519_soft.c
)

if(CONFIG_STCP_V2)
    find_program(CARGO_EXECUTABLE cargo REQUIRED)

    # Prefer an explicit CMake cache/command-line value.  This survives
    # Zephyr sysbuild -> application sub-CMake invocation reliably.
    if(DEFINED STCP_SHARED_RUST_CORE_DIR AND
       NOT "${STCP_SHARED_RUST_CORE_DIR}" STREQUAL "")
        set(STCP_V2_RUST_DIR "${STCP_SHARED_RUST_CORE_DIR}")
    elseif(DEFINED ENV{STCP_SHARED_RUST_CORE_DIR} AND
           NOT "$ENV{STCP_SHARED_RUST_CORE_DIR}" STREQUAL "")
        set(STCP_V2_RUST_DIR "$ENV{STCP_SHARED_RUST_CORE_DIR}")
    elseif(DEFINED STCP_CANONICAL_CORE AND
           NOT "${STCP_CANONICAL_CORE}" STREQUAL "")
        set(STCP_V2_RUST_DIR "${STCP_CANONICAL_CORE}")
    else()
        get_filename_component(STCP_V2_RUST_DIR
            "${CMAKE_CURRENT_LIST_DIR}/../../../kernel/module/rust" ABSOLUTE)
    endif()

    get_filename_component(STCP_V2_RUST_DIR
        "${STCP_V2_RUST_DIR}" ABSOLUTE)

    if(STCP_V2_RUST_DIR MATCHES "/STCPv(2|3)/")
        message(FATAL_ERROR "Refusing legacy STCP Rust core: ${STCP_V2_RUST_DIR}")
    endif()

    message(STATUS "STCPv2 requested canonical Rust core: ${STCP_V2_RUST_DIR}")

    if(NOT EXISTS "${STCP_V2_RUST_DIR}/Cargo.toml")
        message(FATAL_ERROR
            "Canonical STCPv2 Rust core not found: ${STCP_V2_RUST_DIR}. "
            "Set STCP_SHARED_RUST_CORE_DIR if the repository layout differs.")
    endif()

    set(STCP_V2_RUST_TARGET thumbv8m.main-none-eabi)
    set(STCP_V2_RUST_LIB
        ${STCP_V2_RUST_DIR}/target/${STCP_V2_RUST_TARGET}/release/libstcp_kernel_core.a)

    add_custom_command(
        OUTPUT ${STCP_V2_RUST_LIB}
        COMMAND ${CMAKE_COMMAND} -E env
            CARGO_TERM_COLOR=always
            RUSTUP_TOOLCHAIN=${CONFIG_STCP_V2_RUST_TOOLCHAIN}
            ${CARGO_EXECUTABLE} build
                -Z build-std=core,alloc
                --manifest-path ${STCP_V2_RUST_DIR}/Cargo.toml
                --target ${STCP_V2_RUST_TARGET}
                --release
        DEPENDS
            ${STCP_V2_RUST_DIR}/Cargo.toml
            ${STCP_V2_RUST_DIR}/src/lib.rs
            ${STCP_V2_RUST_DIR}/src/ffi.rs
            ${STCP_V2_RUST_DIR}/src/session.rs
            ${STCP_V2_RUST_DIR}/src/carrier.rs
            ${STCP_V2_RUST_DIR}/src/crypto.rs
        WORKING_DIRECTORY ${STCP_V2_RUST_DIR}
        COMMENT "Building canonical shared STCPv2 Rust core for Zephyr"
        VERBATIM
    )

    add_custom_target(stcp_v2_rust_core_build DEPENDS ${STCP_V2_RUST_LIB})
    add_dependencies(stcp_v2 stcp_v2_rust_core_build)
    zephyr_link_libraries(${STCP_V2_RUST_LIB})
    message(STATUS "STCPv2 canonical Rust core: ${STCP_V2_RUST_DIR}")
endif()
