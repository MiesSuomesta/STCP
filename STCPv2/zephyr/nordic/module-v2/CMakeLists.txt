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

    if(DEFINED ENV{STCP_SHARED_RUST_CORE_DIR})
        set(STCP_V2_RUST_DIR "$ENV{STCP_SHARED_RUST_CORE_DIR}")
    else()
        # standalone layout: module-v2 and kernel are siblings under the STCP root
        # canonical shared core lives at STCP_ROOT/kernel/module/rust
        get_filename_component(STCP_V2_RUST_DIR
            "${CMAKE_CURRENT_LIST_DIR}/../kernel/module/rust" ABSOLUTE)
    endif()

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
