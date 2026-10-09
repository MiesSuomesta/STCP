#!/usr/bin/env bash
# Example only. Adapt interface/host to the actual lab fault point.
#
# STCP_FAULT_COMMAND='bash example-fault-hooks.sh down'
# STCP_FAULT_RECOVER_COMMAND='bash example-fault-hooks.sh up'
set -Eeuo pipefail
ACTION="${1:-}"
TARGET="${FAULT_SSH_TARGET:-root@gateway}"
IFACE="${FAULT_IFACE:-eth1}"
case "$ACTION" in
  down) ssh "$TARGET" "ip link set '$IFACE' down" ;;
  up)   ssh "$TARGET" "ip link set '$IFACE' up" ;;
  *) echo "usage: $0 down|up" >&2; exit 2 ;;
esac
