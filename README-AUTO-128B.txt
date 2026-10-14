STCPv7 adaptive AUTO compression patch, based on 2026-10-14 STCP bundle.
Apply at STCP repository root: git apply --check stcpv7-auto-128b.patch && git apply stcpv7-auto-128b.patch
Linux AUTO: 4x32B L1 probe; OFF if poor, else choose L1-L5.
Zephyr AUTO (level 0): same selection. Zephyr fixed levels 1-5 and OFF -1 unchanged.
Existing whole-frame fallback if compression grows payload is retained. No wire changes.
The L1 probe allocates a temporary Vec in the current codec; benchmark heap/CPU on Zephyr.
Thresholds are heuristics, not tuned to measured workloads. Linux current AUTO threshold (default 1024B) remains.
Build and hardware tests NOT run.
