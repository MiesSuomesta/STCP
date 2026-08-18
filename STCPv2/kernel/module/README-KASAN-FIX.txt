STCPv2 canonical Linux kernel module - KASAN debug/fix patch

Based on the canonical linux-kernel/linux-module snapshot from the latest
support bundle supplied on 2026-08-10.

Changes:
  1. src/stcp_proto.c
     - Initialize child socket ssk->carrier = NULL immediately after sk_alloc /
       sock_init_data.  Common accept cleanup paths can run before a carrier is
       attached, so the pointer must never contain stale/uninitialized data.

  2. src/stcp_ops.c
     - Add release-entry printk with socket/STCP context/carrier state.
     - Add ratelimited recvmsg-entry printk with socket/STCP context/carrier
       state, length and flags.

The instrumentation is intentionally small and does not change transport or
handshake logic.  Note that an LSM/AppArmor failure occurring before the
proto_ops recvmsg callback may happen before the recvmsg-entry printk; the
release trace still helps correlate socket lifetime/teardown with the crash.
