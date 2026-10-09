# P2P application split

Bundle layout:

- `application/` — normal STCP regression application, P2P removed.
- `p2p-application/` — dedicated native P2P + Noise Zephyr application.

No Noise protocol behavior was changed. The existing `p2p_benchmark.c` and
`p2p_noise_probe.c` were moved unchanged into the dedicated application.
Only the shell ownership/build configuration was separated.

Recommended installation from `zephyr/nordic`:

1. Keep the current known-good tree/tag available for rollback.
2. Replace the existing `application/` with the bundled `application/`.
3. Add the bundled `p2p-application/` next to it.
4. Build the normal application and run the existing regression suite first.
5. Build/flash P2P separately with `p2p-application/scripts/build.sh` and
   `p2p-application/scripts/flash.sh`.

The normal application should no longer contain `CONFIG_STCP_P2P_BENCH`,
P2P shell commands, or P2P source files.
