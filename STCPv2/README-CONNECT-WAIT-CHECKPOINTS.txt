STCPv2 Zephyr connect wait checkpoint overlay
==============================================

Extract in repository root:

  cd ~/STCP/STCPv2
  unzip -o stcp-zephyr-connect-wait-checkpoint-overlay.zip

Changed:
  zephyr/nordic/module-v2/src/stcp_v2_socket.c

Connect checkpoints:
  C11 core stcp_rust_connect enter
  C12 core stcp_rust_connect return
  C13 RX start enter
  C14 RX start return
  C15 handshake start enter
  C16 handshake start return
  C17 wait_connected enter
  C18 wait_connected return
  C19 successful connect return

wait_connected checkpoints:
  W01 first five is_connected() results
  W02 first five tick() results
  W03 first five semaphore waits

Existing 25-iteration CONNDIAG logging remains in place.

No state-machine, timeout, socket, protocol, crypto or memory behavior is changed.
