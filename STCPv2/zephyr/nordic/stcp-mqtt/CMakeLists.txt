cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(stcp_mqtt)
target_sources(app PRIVATE src/main.c src/mqtt_codec.c src/mqtt_proxy.c)
