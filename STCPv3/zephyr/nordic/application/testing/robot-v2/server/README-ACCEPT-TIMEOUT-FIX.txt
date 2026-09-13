STCPv2 BEN2 server accept timeout fix
=====================================
Treat AF_STCP accept() ETIMEDOUT/EAGAIN/EWOULDBLOCK as transient idle
conditions and retry instead of terminating the benchmark server.

Extract in robot-v2 root:
  unzip -o stcp-v2-bench-server-accept-timeout-fix-20260826.zip
  make -C server clean all
