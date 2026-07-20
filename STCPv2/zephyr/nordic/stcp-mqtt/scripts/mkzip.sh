#!/usr/bin/env bash
set -euo pipefail
DN=$PWD

NORDIC_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic"
APP_DIR="stcp-mqtt"
STCP_MODULE_DIR="stcp-module"

(
	cd $NORDIC_DIR
	zip -r "$DN"/mqtt-paketti.zip $APP_DIR $STCP_MODULE_DIR
)
