STCPv2 recv/release lifetime race fix
Baseline: module-22082026_143739.zip

Changed files:
  include/stcp_socket.h
  src/stcp_proto.c
  src/stcp_ops.c

What changed:
  - per-socket recv_callbacks counter + recv_drain_wq
  - stcp_recvmsg() logs RECV-ENTER/RECV-EXIT and participates in lifetime drain
  - teardown wakes blocked recvmsg() calls and waits until recv_callbacks == 0
  - recvmsg wait condition observes teardown/rust_ctx disappearance
  - release logs RELEASE-WAIT-RECV / RELEASE-RECV-DRAINED / RELEASE-HANDOFF
  - release no longer writes sock->sk = NULL itself
  - after sk_common_release(), release returns immediately without touching sk/ssk

Expected useful trace around close:
  stcp-lifetime: RELEASE-MARK ...
  stcp-lifetime: RELEASE-WAIT-RECV ... active=N
  stcp-lifetime: RECV-EXIT ... active=0 teardown=1 ...
  stcp-lifetime: RELEASE-RECV-DRAINED ... active=0
  ... carrier/Rust teardown ...
  stcp-lifetime: RELEASE-HANDOFF ... recv_active=0

Install as overlay from module root:
  unzip stcp-recv-release-race-fix-20260822.zip
  cp -av stcp-recv-release-race-fix/include/stcp_socket.h include/
  cp -av stcp-recv-release-race-fix/src/stcp_proto.c src/
  cp -av stcp-recv-release-race-fix/src/stcp_ops.c src/

Build note:
  Source structural checks completed. Full host build could not be run in the
  sandbox because this project Makefile requires rustup, which is unavailable
  here.
