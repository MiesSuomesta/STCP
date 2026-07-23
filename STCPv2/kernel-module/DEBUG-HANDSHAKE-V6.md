# STCP handshake/accept debug v6

This package changes TCP accept ordering to call kernel_accept() before creating the Rust child, increases connect/send readiness timeouts to 30 seconds, and adds extensive `stcp-debug-v6` tracing to connect, accept, carrier TX, carrier RX and Rust dispatch paths.

Build and load the same source revision on both hosts. After a failed test collect:

```bash
sudo dmesg | grep stcp-debug-v6
```

Expected TCP sequence:

1. `ACCEPT ENTER`
2. `accept TCP carrier_accept begin/result`
3. `accept TCP rust child create`
4. `accept wire`
5. RX thread starts
6. handshake start
7. carrier_send / TCP TX
8. RX bytes / Rust dispatch
9. ACCEPT COMPLETE and connect handshake complete
