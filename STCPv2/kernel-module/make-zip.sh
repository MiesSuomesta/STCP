#!/bin/bash


bash clean-all.sh
rm -f ../kernel.zip
zip -r ../kernel.zip common-rust/. echo-server/.  raspberry-kernel-module/. stcp-module/. stcp-mqtt/. x86-kernel-module/.
