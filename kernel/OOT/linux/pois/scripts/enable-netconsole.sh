#!/bin/bash

echo 1 | sudo tee /proc/sys/kernel/sysrq

# Jos softlockup/hardlockup -> panic -> reboot (ottaa logit talteen jos pstore/console)
echo 1  | sudo tee /proc/sys/kernel/panic_on_oops
echo 1  | sudo tee /proc/sys/kernel/hardlockup_panic
echo 1  | sudo tee /proc/sys/kernel/softlockup_panic
echo 10 | sudo tee /proc/sys/kernel/panic

# Näytä kaikki
sudo dmesg -n 8
sudo sysctl -w kernel.printk="8 8 1 7"

# Tee varoituksista tappavia (jää pstoreen)
sudo sysctl -w kernel.panic_on_warn=1
sudo sysctl -w kernel.panic_on_oops=1
sudo sysctl -w kernel.panic=10

# Lockupit näkyviin
sudo sysctl -w kernel.softlockup_panic=1
sudo sysctl -w kernel.hung_task_panic=1
sudo sysctl -w kernel.hung_task_timeout_secs=10

nc_make() {
  # Usage:
  #   nc_make "6666@192.168.1.100/90:1b:0e:10:ae:43"
  #   nc_make "6666@192.168.1.100/90:1b:0e:10:ae:43" --apply
  #
  # Output:
  #   modprobe netconsole netconsole=@<SRC_IP>/<IFACE>,<DEST_SPEC>

  local dest_spec="${1:-}"
  local apply="${2:-}"

  if [[ -z "$dest_spec" ]]; then
    echo "Usage: nc_make \"<DST_PORT>@<DST_IP>/<DST_MAC>\" [--apply]" >&2
    return 2
  fi

  # Parse destination: "<port>@<ip>/<mac>"
  local dst_port dst_ip dst_mac
  dst_port="${dest_spec%%@*}"
  local rest="${dest_spec#*@}"
  dst_ip="${rest%%/*}"
  dst_mac="${rest#*/}"

  if [[ -z "$dst_port" || -z "$dst_ip" || -z "$dst_mac" || "$dst_port" == "$dest_spec" ]]; then
    echo "Invalid dest spec. Expected: <DST_PORT>@<DST_IP>/<DST_MAC>" >&2
    echo "Got: $dest_spec" >&2
    return 2
  fi

  # Find route to dst_ip => interface + preferred src ip
  local route
  route="$(ip -o route get "$dst_ip" 2>/dev/null | head -n1)" || true
  if [[ -z "$route" ]]; then
    echo "Failed to resolve route to $dst_ip" >&2
    return 1
  fi

  local iface src_ip
  read src_ip iface < <(ip -4 route get "$dst_ip" | awk '{for(i=1;i<=NF;i++){if($i=="src")src=$(i+1); if($i=="dev")dev=$(i+1)} } END{print src,dev}')
  echo "Got src_fi=$src_ip iface=$iface"

  if [[ -z "$iface" || -z "$src_ip" ]]; then
    echo "Could not parse iface/src from: $route" >&2
    return 1
  fi

  local nc_value="netconsole=@${src_ip}/${iface},${dest_spec}"

  echo "modprobe -r netconsole 2>/dev/null || true"
  echo "modprobe netconsole \\"
  echo "  ${nc_value}"

  if [[ "$apply" == "--apply" ]]; then
    sudo modprobe -r netconsole 2>/dev/null || true
    sudo modprobe netconsole "${nc_value}"
  fi
}

echo 8 | sudo tee /proc/sys/kernel/printk
nc_make "6666@192.168.1.100/90:1b:0e:10:ae:43" --apply

sudo sysctl -w kernel.printk_ratelimit=0
sudo sysctl -w kernel.printk_ratelimit_burst=0

echo "NETCONSOLE ARMED" | sudo tee /dev/kmsg
echo "NETCONSOLE PRINTK RATELIMITS DISABLED @ $(hostname)" | sudo tee /dev/kmsg

echo
echo "[✓] netconsole enabled"
echo
echo "[✓] printk ratelimit OFF"
echo
echo "To receive logs on destination host:"
echo "  nc -klu ${DST_PORT}"
echo
echo "Test with:"
echo "  echo 'NETCONSOLE TEST OK' | tee /dev/kmsg"
