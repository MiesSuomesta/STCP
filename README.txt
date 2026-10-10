STCP UDP follow-up, 2026-10-10

Includes the previous pool-flow-control overlay unchanged, plus a new targeted
UDP carrier atomic-send fix and failure-only AES-GCM PSA diagnostics.

Apply from the active STCP source root:
  python3 pool-flow-fix/apply-pool-flow-fix.py .
  python3 udp-send-fix/apply.py zephyr/nordic/module-v2

If the pool overlay is already installed, its installer is idempotent.
Both installers preserve backups and validate supported source patterns.
If the pool installer rejects an older/different source, stop and supply the
current module; do not overwrite it with the morning source archive.

Rebuild/install host modules for the ACK changes. Pristine rebuild and flash
Zephyr so the changed C sources and Rust pool admission are included.
Run stcp bench all and retain full client and server logs.

New fix: UDP transmits exactly one datagram per frame. Positive short sends
return EIO without sending a second fragment. EINTR retries the whole untouched
datagram. Native errors retain errno; missing errno falls back to EIO.
TCP keeps its existing send-all loop.

Diagnostics: STCP_UDP_TX_FAIL / STCP_UDP_TX_SHORT identify carrier failure;
STCP_GCM_IMPORT_FAIL / STCP_GCM_ENCRYPT_FAIL identify PSA failure including
status and lengths, never keys or plaintext. Diagnostics do not change AES
algorithm, error semantics, memory pool size or wire format.

Validation here: patch applied twice successfully to supplied morning Zephyr
sources; compiled carrier shim tests passed for UDP success/short/error/EINTR
and TCP partial send; existing ASan/UBSan pool pressure tests passed 100 cycles.
No NCS build or device access here. The first-send EIO root cause remains
unconfirmed; these diagnostics make its origin observable, rather than claiming
pool exhaustion from the supplied first-send log.
