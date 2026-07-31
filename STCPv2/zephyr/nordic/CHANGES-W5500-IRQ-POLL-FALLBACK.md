# W5500 IRQ polling fallback

The W5500 debug log showed `S0_IR=0x14` (SENDOK + RECV) while the configured
interrupt GPIO callback did not wake the driver. The Ethernet frame had been
sent successfully, but Zephyr returned `-EIO` because `tx_sem` was never given.

This package patches the NCS 3.3.0 W5500 driver during Ethernet builds:

- normal GPIO interrupt handling remains enabled;
- TX waits up to 500 ms for the normal SENDOK callback;
- if it times out, Socket 0 IR is read directly;
- SENDOK is acknowledged and the TX call succeeds when the hardware reports it;
- the W5500 monitor thread polls Socket 0 IR periodically;
- both SENDOK and RECV are processed through the polling fallback;
- logs distinguish normal timeout, TX polling success and periodic IRQ polling.

This is required because a TX-only fallback would not receive ARP replies or TCP
packets when the same GPIO interrupt path is unavailable.
