# STCP/UDP direct TX burst v13

This version keeps the v10 direct `kernel_sendmsg()` path and RX session fairness.
It does not use the v12 asynchronous shared TX worker.

The synchronous per-session UDP burst is now controlled by the module parameter:

```text
udp_sendmsg_frames=8
```

Allowed range: 1..64 frames. One frame carries 1400 bytes of STCP payload.
The default burst is therefore 11200 bytes.

Runtime tuning:

```bash
cat /sys/module/stcp/parameters/udp_sendmsg_frames
echo 4  | sudo tee /sys/module/stcp/parameters/udp_sendmsg_frames
echo 8  | sudo tee /sys/module/stcp/parameters/udp_sendmsg_frames
echo 16 | sudo tee /sys/module/stcp/parameters/udp_sendmsg_frames
```

Use the same setting on both endpoints when comparing benchmark runs.
