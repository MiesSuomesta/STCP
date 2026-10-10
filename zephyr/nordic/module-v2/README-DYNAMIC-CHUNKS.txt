Extract into zephyr/nordic/module-v2, replacing rust/src/byte_queue.rs. Supersedes 128B/1KiB chunk experiments. Retains existing 16-frame windows and memory diagnostics.

push_slice reserves exact incoming byte count per fresh chunk, capped at 4096. Incoming bytes include STCP wire header, encrypted payload, nonce and authentication tag. A 1081-byte frame requests 1081 bytes, a 17-byte control frame requests 17; partial/coalesced TCP reads use actual read bytes, not assumed whole-frame length. No fixed 4096-byte minimum and no 128-byte fragmentation. Existing chunk growth is also proportional to appended bytes. This is allocation sizing, not a receive-memory budget; metadata and contiguous frame extraction can require separate allocations.

Rebuild Rust core and Zephyr application, flash, run FULL first after reboot. Rust/Zephyr compilation unavailable here; target test required. Provide first STCP_MEM block if failure persists.
