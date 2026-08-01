# STCP blocking connect flag and diagnostics

This package fixes the benchmark stopping before `connect()` when AF_STCP
returns `ENOTSUP` for `fcntl(F_GETFL/F_SETFL)`.

## Changes

- Added `CONFIG_BENCH_STCP_BLOCKING_CONNECT`.
- STCP sockets skip `O_NONBLOCK` setup when the flag is enabled.
- Native TCP keeps the original non-blocking connect + poll path.
- Added explicit socket mode/feature logs.
- Added best-effort send and receive socket timeouts before connect.
- Cleared stale errno before socket creation and no longer reports stale errno
  after a successful `socket()` call.
- Existing carrier-connect and STCP-offload diagnostics remain enabled.
- Existing `stcp ping` shell command remains included.

Expected successful STCP path:

```text
APP SOCKET RETURN fd=1 success
APP SOCKET FLAGS transport=stcp mode=blocking nonblock_setup=skipped feature=BENCH_STCP_BLOCKING_CONNECT
APP ZSOCK_CONNECT CALL fd=1 ... mode=blocking
CONNECT ENTER ...
CALLING CARRIER CONNECT fd=0
CARRIER CONNECT RETURN fd=0 rc=0 errno=0
CONNECT SUCCESS ...
APP ZSOCK_CONNECT RETURN fd=1 rc=0 mode=blocking
```
