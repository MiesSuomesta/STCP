#!/bin/bash
set -e

findCp() {
	tgt="$1"
	to="$2"
	cp $(find . -name "$tgt" | head -n 1) "$to"
	if [ $? -eq 0 ]; then
		echo "[+]   $tgt => $to... OK"
	else
		echo "[+]   $tgt => $to... NOK"
	fi
}

echo "[+] Rebuilding SDK directory..."
rm -rf sdk
mkdir -p sdk/include
mkdir -p sdk/lib
mkdir -p sdk/cmake
mkdir -p sdk/examples/client-demo
mkdir -p sdk/examples/server-demo

echo "[+] Copying headers..."
# Copy and rename headers

cp stcp-client-lib/rust-c-wrapper/include/stcp_client_cwrapper_lib.h sdk/include/stcp_client.h
cp stcp-server-lib/rust-c-wrapper/include/stcp_server_cwrapper_lib.h sdk/include/stcp_server.h

echo "[+] Copying libraries..."
# Collect static libraries from target directory
findCp "libstcpclient*.a" sdk/lib/libstcpclient.a
findCp "libstcpserver*.a" sdk/lib/libstcpserver.a

echo "[+] Copying examples..."
cp stcp-client-lib/rust-c-wrapper/demo/main.c sdk/examples/client-demo/
cp stcp-server-lib/rust-c-wrapper/demo/main.c sdk/examples/server-demo/

echo "[+] Creating CMakeLists.txt..."
cat <<EOF > sdk/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(stcp_sdk_demo C)

set(CMAKE_C_STANDARD 99)

include_directories(include)
link_directories(lib)

option(TESTING "Run symbol check for exported SDK functions" OFF)

if(TESTING)
    message(STATUS "🧪 Testing that all symbols are found that are in headers...")
    include(cmake/symbol_test.cmake)
    run_symbol_test("\${CMAKE_SOURCE_DIR}/include" "\${CMAKE_SOURCE_DIR}/lib")
endif()

add_executable(client examples/client-demo/main.c)
target_link_libraries(client stcpclient pthread m)

add_executable(server examples/server-demo/main.c)
target_link_libraries(server stcpserver pthread m)
EOF

echo "[+] Creating CMake test-files..."
cat <<EOF > sdk/cmake/symbol_test.cmake
function(run_symbol_test include_dir lib_dir)
    file(GLOB HEADER_FILES "\${include_dir}/*.h")

    foreach(header \${HEADER_FILES})
        file(READ "\${header}" HEADER_CONTENTS)
        string(REGEX MATCHALL "stcp_[a-zA-Z0-9_]*" FOUND_FUNCS "\${HEADER_CONTENTS}")

        foreach(func \${FOUND_FUNCS})
            set(SYMBOL_FOUND FALSE)

            foreach(libfile client server)
                execute_process(
                    COMMAND nm -g "\${lib_dir}/libstcp\${libfile}.a"
                    COMMAND grep "\${func}"
                    RESULT_VARIABLE nm_result
                    OUTPUT_QUIET
                    ERROR_QUIET
                )
                if(nm_result EQUAL 0)
                    set(SYMBOL_FOUND TRUE)
                endif()
            endforeach()

            if(NOT SYMBOL_FOUND)
                message(FATAL_ERROR "❌ Function \${func} NOK, it does not found in any library!")
            else()
                message(STATUS "✅ Function \${func} OK")
            endif()
        endforeach()
    endforeach()
endfunction()
EOF

echo "[+] Creating README.txt..."
cat <<EOF > sdk/README.txt
STCP SDK 1.0.0

Sisältää:
- include/: headerit (stcp_client.h, stcp_server.h)
- lib/: staattiset .a-kirjastot
- examples/: C-kieliset esimerkit clientille ja serverille
- CMakeLists.txt: helppo buildi esimerkeille
EOF

echo "[✓] SDK package created under: sdk/"

# Luo pkg-config-tiedosto
mkdir -p sdk/pkgconfig
cat > sdk/pkgconfig/stcp-sdk.pc <<EOF
prefix=/usr/local
exec_prefix=\${prefix}
includedir=\${prefix}/include
libdir=\${exec_prefix}/lib

Name: STCP SDK
Description: Secure Transport Control Protocol (STCP) C SDK
Version: 1.0.0
Cflags: -I\${includedir}
Libs: -L\${libdir} -lstcp_client -lstcp_server
EOF

# Lisää installeri
cat > sdk/install-sdk.sh <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

PREFIX="${1:-/usr/local}"

echo "📦 Installing STCP SDK with prefix: $PREFIX"

install -d "${PREFIX}/include"
install -d "${PREFIX}/lib"
install -d "${PREFIX}/lib/pkgconfig"

cp -v include/*.h "${PREFIX}/include/"
cp -v lib/*.a "${PREFIX}/lib/"
cp -v pkgconfig/stcp-sdk.pc "${PREFIX}/lib/pkgconfig/"

echo "✅ Installation complete!"
echo ""
echo "Use with pkg-config:"
echo "  PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig pkg-config --cflags --libs stcp-sdk"
EOF

chmod +x sdk/install-sdk.sh
