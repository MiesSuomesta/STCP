cmake_minimum_required(VERSION 3.16)
project(STCP-client-library VERSION 0.1.0 LANGUAGES C)

set(RUST_TARGET x86_64-unknown-linux-musl)
set(RUST_LIB_NAME stcp_client_cwrapper_lib)

set(RUST_WRAPPER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/rust-c-wrapper)
set(RUST_WRAPPER_RELEASE_PATH ${RUST_WRAPPER_DIR}/target/${RUST_TARGET}/release)

set(RUST_LIB_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib/lib${RUST_LIB_NAME}.a)
set(RUST_HEADER_PATH ${CMAKE_CURRENT_BINARY_DIR}/include/${RUST_LIB_NAME}.h)

add_custom_command(
    OUTPUT ${RUST_LIB_PATH} ${RUST_HEADER_PATH}
    WORKING_DIRECTORY ${RUST_WRAPPER_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/lib
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include
    COMMAND cargo build --release --target=${RUST_TARGET}
    COMMAND cbindgen --config cbindgen.toml --crate ${RUST_LIB_NAME} --output ${RUST_HEADER_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy
            ${RUST_WRAPPER_RELEASE_PATH}/lib${RUST_LIB_NAME}.a
            ${RUST_LIB_PATH}
    COMMENT "Building Rust wrapper and generating header"
    DEPENDS ${RUST_WRAPPER_DIR}/src/lib.rs ${RUST_WRAPPER_DIR}/Cargo.toml
)

add_custom_target(rustlib_build ALL
    DEPENDS ${RUST_LIB_PATH} ${RUST_HEADER_PATH}
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Rust wrapper -kirjaston polku
set(RUST_WRAPPERLIB_PATH ${CMAKE_CURRENT_BINARY_DIR}/lib/libstcp_client_cwrapper_lib.a)

add_executable(stcp_client src/main.c)
add_dependencies(stcp_client rustlib_build)

target_include_directories(stcp_client PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
target_link_libraries(stcp_client
    ${RUST_WRAPPERLIB_PATH}
    pthread dl m
)

# Debug-tulosteet
message(STATUS "Rust wrapper .a: ${RUST_WRAPPERLIB_PATH}")
message(STATUS "Header: ${RUST_HEADER_PATH}")

