# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/zephyr/samples/hello_world")
  file(MAKE_DIRECTORY "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/zephyr/samples/hello_world")
endif()
file(MAKE_DIRECTORY
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix/tmp"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix/src/hello_world-stamp"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix/src"
  "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix/src/hello_world-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix/src/hello_world-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/_sysbuild/sysbuild/images/hello_world-prefix/src/hello_world-stamp${cfgdir}") # cfgdir has leading slash
endif()
