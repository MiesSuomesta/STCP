
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(stcp_zephyr_app)

# ------------------------------------------------------------
# App sources
# ------------------------------------------------------------
message("Tarkistus")
if(CONFIG_MQTT_LIB_STCP)
    message("Buildissa mukana ok")
    target_sources(app PRIVATE
        src/main.c
        src/stcp_mqtt.c
        src/stcp_mqtt_fsm.c
    )
    target_link_libraries(app PRIVATE
        ${WEST_TOPDIR}/modules/stcp/rust/target/thumbv8m.main-none-eabi/release/libthe_stcp_kernel_module.a
    )
endif()

# ------------------------------------------------------------
# Optional debug flag pass-through
# ------------------------------------------------------------
zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../moduuli-tcp/include
    ${CMAKE_BINARY_DIR}/modules/moduuli-tcp
)

if(DEFINED STCP_DEBUG)
    target_compile_definitions(app PRIVATE STCP_DEBUG=${STCP_DEBUG})
endif()

##add_compile_definitions( Os )
target_include_directories(app PRIVATE include)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_custom_target(force_stcp_gen ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/modules/moduuli-tcp/stcp
    COMMAND cbindgen
        --config ${WEST_TOPDIR}/modules/stcp/rust/cbindgen.toml
        --crate the_stcp_kernel_module
        --output ${CMAKE_BINARY_DIR}/modules/moduuli-tcp/stcp/stcp_generated.h
    WORKING_DIRECTORY ${WEST_TOPDIR}/modules/stcp/rust
)
add_dependencies(app force_stcp_gen)

zephyr_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../moduuli-tcp/include
    ${CMAKE_BINARY_DIR}/modules/moduuli-tcp
    ${CMAKE_CURRENT_BINARY_DIR}
)
