# W5500 low-latency polling fallback

The previous fallback proved that Ethernet TX and RX work, but the 500 ms TX
wait and monitor-period RX polling limited throughput to only a few kbit/s.

This revision keeps the ordinary GPIO interrupt path and adds a low-latency
fallback:

- TX waits in 1 ms steps and checks Socket 0 IR after each step;
- SENDOK is normally detected within 1-2 ms instead of after 500 ms;
- RECV events discovered during TX are serviced immediately;
- the W5500 worker polls Socket 0 IR every 1 ms;
- ARP replies, TCP ACKs and payload receive no longer wait for the normal
  monitor period;
- link status is checked every second rather than on every polling cycle;
- debug counters report GPIO events, TX events, RX events and polling count;
- a full register dump remains available after a real 50 ms TX timeout.

The SPI clock remains at 4 MHz while the interrupt mapping is investigated.
After stable benchmark runs, it can be raised in stages.
