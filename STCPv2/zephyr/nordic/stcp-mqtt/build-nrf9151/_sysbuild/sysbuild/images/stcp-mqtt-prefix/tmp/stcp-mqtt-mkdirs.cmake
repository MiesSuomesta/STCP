# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt")
  file(MAKE_DIRECTORY "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt")
endif()
file(MAKE_DIRECTORY
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/stcp-mqtt"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix/tmp"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix/src/stcp-mqtt-stamp"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix/src"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix/src/stcp-mqtt-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix/src/stcp-mqtt-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-mqtt/build-nrf9151/_sysbuild/sysbuild/images/stcp-mqtt-prefix/src/stcp-mqtt-stamp${cfgdir}") # cfgdir has leading slash
endif()
