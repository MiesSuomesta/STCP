# Install script for directory: /home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "MinSizeRel")
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
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/lib/backtrace/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/lib/ext/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/lib/fih/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/lib/tfm_log/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/lib/tfm_log_unpriv/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/lib/tfm_vprintf/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/tools/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/secure_fw/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/interface/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/platform/cmake_install.cmake")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/platform/ext/accelerator/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE DIRECTORY MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/bin/")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/lib/s_veneers.o")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/lib" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/secure_fw/s_veneers.o")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/client.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/error.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/psa/client.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/psa/error.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa_manifest/sid.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa_manifest" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/generated/interface/include/psa_manifest/sid.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/config_impl.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/generated/interface/include/config_impl.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_veneers.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_ns_interface.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_veneers.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_ns_interface.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_ns_client_ext.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_ns_client_ext.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/config_tfm.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/secure_fw/include/config_tfm.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/config_base.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/config/config_base.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_hybrid_platform.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/secure_fw/spm/include/tfm_hybrid_platform.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_psa_call_pack.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_psa_call_pack.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/framework_feature.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/generated/interface/include/psa/framework_feature.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/protected_storage.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/storage_common.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/psa/protected_storage.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/psa/storage_common.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_ps_defs.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_ps_defs.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/internal_trusted_storage.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/storage_common.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/psa/internal_trusted_storage.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/psa/storage_common.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_its_defs.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_its_defs.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/build_info.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_adjust_auto_enabled.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_adjust_config_key_pair_types.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_adjust_config_synonyms.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_compat.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_driver_common.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_extra.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_legacy.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_platform.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_se_driver.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_sizes.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_struct.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_types.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_values.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_driver_contexts_key_derivation.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_driver_contexts_primitives.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa/crypto_driver_contexts_composites.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/psa" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/build_info.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_adjust_auto_enabled.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_adjust_config_key_pair_types.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_adjust_config_synonyms.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_compat.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_driver_common.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_extra.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_legacy.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_platform.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_se_driver.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_sizes.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_struct.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_types.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto_values.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/psa/crypto.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/nrf/subsys/nrf_security/include/psa/crypto_driver_contexts_key_derivation.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/nrf/subsys/nrf_security/include/psa/crypto_driver_contexts_primitives.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/nrf/subsys/nrf_security/include/psa/crypto_driver_contexts_composites.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/mbedtls/build_info.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/mbedtls/config_psa.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/mbedtls" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/mbedtls/build_info.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/crypto/oberon-psa-crypto/include/mbedtls/config_psa.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/nrf-config.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/nrf-psa-crypto-config.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/generated/interface_nrf_security_psa/nrf-config.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/generated/interface_nrf_security_psa/nrf-psa-crypto-config.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_crypto_defs.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_crypto_defs.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/tfm_platform_api.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/tfm_platform_api.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src/tfm_tz_psa_ns_api.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/src/tfm_tz_psa_ns_api.c")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include/os_wrapper")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/include" TYPE DIRECTORY MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/include/os_wrapper")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src/os_wrapper")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src" TYPE DIRECTORY MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/src/os_wrapper")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src/tfm_ps_api.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/src/tfm_ps_api.c")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src/tfm_its_api.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/src/tfm_its_api.c")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src/tfm_crypto_api.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/src/tfm_crypto_api.c")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src/tfm_platform_api.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/interface/src" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/interface/src/tfm_platform_api.c")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/config/cp_check.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/config" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/config/cp_check.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  MESSAGE("----- Installing platform NS -----")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/cmsis/Include;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/cmsis/Include")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/cmsis" TYPE DIRECTORY MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/hal/cmsis_6/CMSIS/Core/Include"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/hal/cmsis_6/CMSIS/Driver/Include"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/common/picolibc.c")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/common" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/common/picolibc.c")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/common/uart_stdout.c;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/common/uart_stdout.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/ext/common" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/common/uart_stdout.c"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ext/common/uart_stdout.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/include")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform" TYPE DIRECTORY MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/include")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/CMakeLists.txt")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns" TYPE FILE MESSAGE_NEVER RENAME "CMakeLists.txt" FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/cmake/spe-CMakeLists.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/toolchain_ns_GNUARM.cmake;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/toolchain_ns_ARMCLANG.cmake;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/toolchain_ns_IARARM.cmake;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/toolchain_ns_CLANG.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ns/toolchain_ns_GNUARM.cmake"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ns/toolchain_ns_ARMCLANG.cmake"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ns/toolchain_ns_IARARM.cmake"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/ns/toolchain_ns_CLANG.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/include/fih.h;/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/include/tfm_plat_ns.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/platform/include" TYPE FILE MESSAGE_NEVER FILES
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/lib/fih/inc/fih.h"
    "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/platform/include/tfm_plat_ns.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/CMakeFiles/tfm_config.dir/install-cxx-module-bmi-MinSizeRel.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/secure_fw/partitions/crypto/CMakeFiles/psa_crypto_config.dir/install-cxx-module-bmi-MinSizeRel.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  include("/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/interface/CMakeFiles/psa_interface.dir/install-cxx-module-bmi-MinSizeRel.cmake" OPTIONAL)
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/spe_export.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/spe_export.cmake"
         "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/CMakeFiles/Export/a6875993bd1f9816d0953020e8fc2d6e/spe_export.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/spe_export-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/spe_export.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/spe_export.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/CMakeFiles/Export/a6875993bd1f9816d0953020e8fc2d6e/spe_export.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake/set_extensions.cmake")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/api_ns/cmake" TYPE FILE MESSAGE_NEVER FILES "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/modules/tee/tf-m/trusted-firmware-m/cmake/set_extensions.cmake")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/home/pomo/git/STCP/STCPv2/zephyr/nordic/ncs-3.3.0/build-hello/hello_world/tfm/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
