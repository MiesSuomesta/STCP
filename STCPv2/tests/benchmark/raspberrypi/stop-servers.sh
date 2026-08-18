#!/usr/bin/env bash
D="$(cd "$(dirname "$0")" && pwd)"; for m in tcp tls stcp; do [ -f "$D/run/$m.pid" ] && kill "$(cat "$D/run/$m.pid")" 2>/dev/null || true; rm -f "$D/run/$m.pid"; done
