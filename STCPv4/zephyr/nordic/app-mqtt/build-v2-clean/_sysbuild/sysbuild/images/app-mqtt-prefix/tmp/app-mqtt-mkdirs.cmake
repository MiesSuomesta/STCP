# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/pomo/zephyr-stcp/stcp/app-mqtt")
  file(MAKE_DIRECTORY "/home/pomo/zephyr-stcp/stcp/app-mqtt")
endif()
file(MAKE_DIRECTORY
  "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/app-mqtt"
  "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix"
  "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix/tmp"
  "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix/src/app-mqtt-stamp"
  "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix/src"
  "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix/src/app-mqtt-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix/src/app-mqtt-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pomo/zephyr-stcp/stcp/app-mqtt/build-v2-clean/_sysbuild/sysbuild/images/app-mqtt-prefix/src/app-mqtt-stamp${cfgdir}") # cfgdir has leading slash
endif()
