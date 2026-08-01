# Ping network readiness fix

- `stcp ping` waits up to 15 seconds for the default interface to be administratively up and carrier up.
- DNS resolution and ICMP transmission start only after Ethernet is ready.
- Returns `-ENETDOWN` with interface state if readiness times out.
- Ethernet build preserves the currently installed W5500 driver and does not patch it.
