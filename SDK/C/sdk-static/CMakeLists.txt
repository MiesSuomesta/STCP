cmake_minimum_required(VERSION 3.10)
project(stcp_sdk_demo C)

set(CMAKE_C_STANDARD 99)

include_directories(include)
link_directories(lib)

option(TESTING "Run symbol check for exported SDK functions" OFF)

if(TESTING)
    message(STATUS "🧪 Testing that all symbols are found that are in headers...")
    include(cmake/symbol_test.cmake)
    run_symbol_test("${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/lib")
endif()

add_executable(client examples/client-demo/main.c)
target_link_libraries(client stcpclient pthread m)

add_executable(server examples/server-demo/main.c)
target_link_libraries(server stcpserver pthread m)
