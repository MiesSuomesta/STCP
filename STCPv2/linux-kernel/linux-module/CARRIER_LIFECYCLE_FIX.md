# Carrier lifecycle fix

The RX kthread previously exited by itself when `kernel_recvmsg()` returned
zero, `-ESHUTDOWN`, `-ENOTCONN`, or `-ECONNRESET`. The carrier retained the old
`task_struct *` and later passed it to `kthread_stop()`, causing a stale-pointer
write and a KASAN/Oops failure in `stcp_carrier_put_root()`.

Changes in `src/stcp_carrier.c`:

- Added `lifecycle_lock` and `stopping` to each carrier.
- Receiver creation and teardown are serialized.
- The task pointer is detached exactly once under the lifecycle lock.
- The RX thread exits only when `kthread_should_stop()` is true.
- Socket shutdown wakes blocking `kernel_recvmsg()` before `kthread_stop()`.
- Even an RX-buffer allocation failure keeps the kthread alive until stop.

Reboot after the previous kernel panic before testing this build.
Recommended tests:

```bash
make LLVM=1 clean
make LLVM=1 V=1 module
make LLVM=1 test
./stress/stcp-stress
```
