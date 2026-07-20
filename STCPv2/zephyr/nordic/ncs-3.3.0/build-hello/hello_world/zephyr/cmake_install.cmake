# Install script for directory: /home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/zephyr

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
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
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/arch/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/lib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/soc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/boards/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/subsys/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/drivers/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/nrf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/mcuboot/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/mbedtls/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/trusted-firmware-m/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/cjson/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/azure-sdk-for-c/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/cirrus-logic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/openthread/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/memfault-firmware-sdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/hostap/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/canopennode/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/chre/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/cmsis/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/cmsis-dsp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/cmsis-nn/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/cmsis_6/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/fatfs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/hal_nordic/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/hal_st/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/hal_tdk/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/hal_wurthelektronik/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/liblc3/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/libmetal/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/libsbc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/littlefs/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/loramac-node/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/lvgl/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/mipi-sys-t/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/nanopb/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/nrf_wifi/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/open-amp/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/percepio/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/picolibc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/segger/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/uoscore-uedhoc/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/zcbor/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/nrfxlib/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/nrf_hw_models/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/modules/connectedhomeip/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/kernel/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/cmake/flash/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/cmake/usage/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/cmake/reports/cmake_install.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/zephyr/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
