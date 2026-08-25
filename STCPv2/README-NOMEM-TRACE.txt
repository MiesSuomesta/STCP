STCPv2 Zephyr targeted -ENOMEM trace overlay
=============================================

Extract directly in the STCPv2 repository root:

    cd ~/STCP/STCPv2
    unzip -o stcp-zephyr-nomem-trace-overlay.zip

This overlay carries forward the latest client-first handshake, stream-drain,
ByteQueue, and direct RX frame-processing fixes. It adds only lightweight
debug events to the remaining receive-side -ENOMEM paths.

Files:
  kernel/module/rust/src/state.rs
  kernel/module/rust/src/session.rs
  kernel/module/rust/src/byte_queue.rs
  kernel/module/rust/src/carrier.rs
  zephyr/nordic/module-v2/src/stcp_v2_rx.c

NOMEM event map:
  event=9001  incoming carrier bytes -> wire ByteQueue push failed
              arg0=input bytes, arg1=errno magnitude

  event=9002  complete wire frame payload extraction failed
              arg0=payload length, arg1=packet type

  event=9003  out-of-order frame queue reserve failed
              arg0=buffered count, arg1=frame sequence

  event=9004  decrypted plaintext -> application ByteQueue publication failed
              arg0=frame sequence, arg1=current app queue length

  event=9090  carrier_receive returned -ENOMEM
              arg0=input bytes, arg1=socket state

  event=9091  fill_application_buffer propagated -ENOMEM

Useful grep after the failing run:

    dmesg | grep -E 'event=900[1-4]|event=909[01]|carrier_receive rc=-12'

No hexdumps, no per-loop logging, and no additional sleeps/barriers are added.
