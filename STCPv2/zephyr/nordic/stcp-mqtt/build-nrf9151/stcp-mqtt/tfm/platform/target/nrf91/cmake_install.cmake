# Install script for directory: /home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/target/nordic_nrf/common/nrf91

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/pomo/zephyr-sdk-0.16.8/arm-zephyr-eabi/bin/arm-zephyr-eabi-objdump")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/platform/target/nrf91/nrf_common/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns/platform/common/nrf91/CMakeLists.txt;/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns/platform/common/nrf91/config.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns/platform/common/nrf91" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/target/nordic_nrf/common/nrf91/ns/CMakeLists.txt"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/target/nordic_nrf/common/nrf91/config.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns/platform/common/nrf91/tests;/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns/platform/common/nrf91/partition")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/api_ns/platform/common/nrf91" TYPE DIRECTORY MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/target/nordic_nrf/common/nrf91/tests"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/target/nordic_nrf/common/nrf91/partition"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt/tfm/platform/target/nrf91/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
