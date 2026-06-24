cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(stcp_zephyr_app)

# ------------------------------------------------------------
# App sources
# ------------------------------------------------------------

target_sources(app PRIVATE
    src/stcp_mqtt.c
    src/main.c
)

# ------------------------------------------------------------
# Optional debug flag pass-through
# ------------------------------------------------------------

if(DEFINED STCP_DEBUG)
    target_compile_definitions(app PRIVATE STCP_DEBUG=${STCP_DEBUG})
endif()

target_include_directories(app PRIVATE include)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
