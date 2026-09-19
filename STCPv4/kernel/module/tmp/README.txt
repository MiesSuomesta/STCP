STCPv4 carrier TX benchmark overlay

Run from STCPv4/kernel/module:
  python3 /path/to/apply.py

Adds aggregate benchmark labels (no hot-path printk):
  C:CARRIER_TCP_TX:TOTAL
  C:CARRIER_TCP_TX:LIFECYCLE_LOCK_WAIT
  C:CARRIER_TCP_TX:KERNEL_SEND
  C:CARRIER_UDP_TX:KERNEL_SEND

TCP TOTAL starts immediately before lifecycle_lock acquisition and ends after
active_sends is released. KERNEL_SEND is recorded per kernel_sendmsg iteration.
UDP KERNEL_SEND is a control measurement.

Requires the existing stcp_kernel_benchmark_* implementation in stcp_memory.c.
The patcher adds extern declarations to stcp_carrier.c if they are absent.
