cmake_minimum_required(VERSION 3.10)
project(stcp_sdk_demo C)

set(CMAKE_C_STANDARD 99)


set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(TOOLCHAIN_PREFIX "arm-linux-musleabi")

# Setup CMake to cross compile........


set(TOOLCHAIN_ROOT "${CMAKE_SOURCE_DIR}/../tools/musl-cross-compiled-libs")
set(TOOLCHAIN_BIN_PATH "${TOOLCHAIN_ROOT}/bin")
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}-gcc")
set(CMAKE_LD "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}-gcc")
set(CMAKE_NM "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}-nm")
set(CMAKE_AR "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}-ar")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN_PATH}/${TOOLCHAIN_PREFIX}-ranlib")

set(CMAKE_LINKER ${CMAKE_C_COMPILER} CACHE FILEPATH "Force linker" FORCE)

# Vältetään hostin linkkeriä
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

include_directories(include)
link_directories(lib)

option(TESTING "Run symbol check for exported SDK functions" OFF)

if(TESTING)
    message(STATUS "🧪 Testing that all symbols are found that are in headers...")
    include(cmake/symbol_test.cmake)
    run_symbol_test("${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/lib")
endif()

#add_executable(client examples/client-demo/main.c)
#target_link_libraries(client stcpclient pthread m)

#add_executable(server examples/server-demo/main.c)
#target_link_libraries(server stcpserver pthread m)
