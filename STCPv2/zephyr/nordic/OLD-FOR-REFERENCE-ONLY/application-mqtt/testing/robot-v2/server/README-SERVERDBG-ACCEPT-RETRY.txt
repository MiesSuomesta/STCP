STCPv2 BEN2 debug server + AF_STCP accept retry
================================================

Combined server build:
- preserves BEN2 upload/download/full-duplex protocol
- keeps SERVERDBG accept/connection/I/O/close diagnostics
- treats accept() EINTR as retry
- treats AF_STCP ETIMEDOUT/EAGAIN/EWOULDBLOCK as transient idle conditions
  and retries accept instead of terminating the server

Install from robot-v2 root:
  unzip -o stcp-v2-bench-server-debug-accept-retry-overlay-20260826.zip
  make -C server clean all

Expected idle marker:
  SERVERDBG ... ACCEPT TRANSIENT ... errno=110(Connection timed out) retry=1

Expected next connection:
  SERVERDBG ... ACCEPT ENTER ... next_cid=2
  SERVERDBG ... ACCEPT RETURN cid=2 ...
