STCPv2 ByteQueue retain-last-chunk overlay
==========================================

Apply:
  cd ~/STCP/STCPv2
  unzip -o stcp-zephyr-bytequeue-retain-last-chunk-overlay.zip

Changed:
  kernel/module/rust/src/byte_queue.rs

Observed failure:
  carrier_receive rc=-12 nomem_stage=9001

Fix:
  Fully consumed older chunks are still released normally, but the final
  remaining ByteQueue chunk is cleared and retained. Vec capacity therefore
  survives an empty queue and can be reused by trailing ACK/CLOSE traffic
  without another heap allocation.

Modified consumed-front sites: 3

No protocol, crypto, FFI or queue-limit behavior is changed.
