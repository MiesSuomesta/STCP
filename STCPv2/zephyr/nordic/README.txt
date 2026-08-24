Synchronous printk-only diagnostics for the AF_STCP create/allocation crash.

No control-flow changes.
Markers:
  V2P CREATE A/B/C
  V2P ALLOC 00..14

After installing, force a pristine/touched rebuild because previous ZIP mtimes
caused Ninja to reuse stale objects.
