message(STATUS "Loading STCP2 offload module from ${CMAKE_CURRENT_LIST_DIR}")
zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
zephyr_library_named(stcp)
zephyr_library_sources_ifdef(CONFIG_STCP
    src/stcp_context.c
    src/stcp_carrier.c
    src/stcp_socket_offload.c
)

if(CONFIG_STCP_RUST_CORE)
    zephyr_library_sources(
        src/stcp_rust_platform.c
        src/stcp_rust_rx.c
    )

    find_program(CARGO_EXECUTABLE cargo REQUIRED)
    set(STCP_RUST_DIR ${CMAKE_CURRENT_LIST_DIR}/rust)
    set(STCP_RUST_TARGET thumbv8m.main-none-eabi)
    set(STCP_RUST_PROFILE release)
    set(STCP_RUST_LIB ${STCP_RUST_DIR}/target/${STCP_RUST_TARGET}/${STCP_RUST_PROFILE}/libstcp_kernel_core.a)

    add_custom_command(
        OUTPUT ${STCP_RUST_LIB}
        COMMAND ${CMAKE_COMMAND} -E env
            CARGO_TERM_COLOR=always
            RUSTUP_TOOLCHAIN=${CONFIG_STCP_RUST_TOOLCHAIN}
            ${CARGO_EXECUTABLE} build
                -Z build-std=core,alloc
                --manifest-path ${STCP_RUST_DIR}/Cargo.toml
                --target ${STCP_RUST_TARGET}
                --release
        DEPENDS
            ${STCP_RUST_DIR}/Cargo.toml
            ${STCP_RUST_DIR}/.cargo/config.toml
            ${STCP_RUST_DIR}/src/lib.rs
            ${STCP_RUST_DIR}/src/ffi.rs
            ${STCP_RUST_DIR}/src/session.rs
            ${STCP_RUST_DIR}/src/carrier.rs
            ${STCP_RUST_DIR}/src/crypto.rs
        WORKING_DIRECTORY ${STCP_RUST_DIR}
        COMMENT "Building shared Raspberry Pi Rust STCP core for ${STCP_RUST_TARGET}"
        VERBATIM
    )
    add_custom_target(stcp_rust_core_build DEPENDS ${STCP_RUST_LIB})
    add_dependencies(stcp stcp_rust_core_build)
    zephyr_link_libraries(${STCP_RUST_LIB})
    message(STATUS "STCP Rust core enabled: ${STCP_RUST_LIB}")
endif()
