# STCP/UDP bounded control priority v17

Baseline: v13 direct TX burst (`udp_sendmsg_frames=24`).

The shared UDP listener previously drained the entire ACK/control queue before
servicing any DATA frame. With many active sessions, continuous 40-byte ACK
traffic could starve DATA in the root dispatcher for tens of seconds even when
there were no socket or queue drops.

v17 keeps ACK/control traffic high priority, but after 64 consecutive control
frames it dispatches one queued DATA frame and then resumes control priority.
FIFO ordering within each session remains unchanged.

Runtime parameter on both x86 and Raspberry builds:

```bash
cat /sys/module/stcp/parameters/udp_rx_control_budget
echo 64 | sudo tee /sys/module/stcp/parameters/udp_rx_control_budget
```

`0` restores the old unlimited strict-priority behaviour. Suggested tuning
values are 32, 64 and 128; start with 64.
