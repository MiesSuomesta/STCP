# X25519 software backend header/include fix

- Ensures both files are included in the package:
  - `stcp-module/src/stcp_x25519_soft.c`
  - `stcp-module/include/stcp/stcp_x25519_soft.h`
- Uses the module include root correctly:
  - `#include <stcp/stcp_x25519_soft.h>`
- Keeps `zephyr_include_directories(${CMAKE_CURRENT_LIST_DIR}/include)`.
- Adds the X25519 source/header to the Rust-core custom build dependencies so changes force a rebuild.
- Overlays the latest available CMake, Kconfig and application configuration files from `zephyr_viimesin.zip` onto the last complete Rust/Zephyr integration tree.
