#!/usr/bin/env bash
set -Eeuo pipefail
APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
BUILD_DIR="${STCP_MQTT_BUILD_DIR:-$APP_ROOT/build-mqtt}"
exec west flash -d "$BUILD_DIR" "$@"
