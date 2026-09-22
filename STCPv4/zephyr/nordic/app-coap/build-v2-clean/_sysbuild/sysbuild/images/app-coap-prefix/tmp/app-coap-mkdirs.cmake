# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/pomo/zephyr-stcp/stcp/app-coap")
  file(MAKE_DIRECTORY "/home/pomo/zephyr-stcp/stcp/app-coap")
endif()
file(MAKE_DIRECTORY
  "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/app-coap"
  "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix"
  "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix/tmp"
  "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix/src/app-coap-stamp"
  "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix/src"
  "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix/src/app-coap-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix/src/app-coap-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pomo/zephyr-stcp/stcp/app-coap/build-v2-clean/_sysbuild/sysbuild/images/app-coap-prefix/src/app-coap-stamp${cfgdir}") # cfgdir has leading slash
endif()
