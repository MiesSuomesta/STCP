# UDP session demultiplexing fix

The UDP listener previously looked up sessions only by:

- listener context
- STCP connection ID

It checked the peer address and port only after the lookup. Connection IDs are
not globally unique across remote hosts, module reloads or reboots, so a valid
frame could match another peer's entry and then be silently discarded.

The demultiplexer now keys sessions by the full tuple:

- listener context
- connection ID
- peer IPv4 address
- peer UDP port

Additional fixes:

- The session registry remains locked while a located raw child pointer is
  dispatched, preventing release from unregistering and freeing the child
  between lookup and use.
- New-session creation is serialized and rechecked to prevent two simultaneous
  PublicKey frames from creating duplicate children.
- Unknown non-PublicKey tuples continue to be dropped until a PublicKey opens
  the session.
- `clean-all.sh` now cleans only STCP-owned build directories and never walks
  Raspberry/Linux kernel source or Nordic SDK trees.

Primary changed file: `common-rust/src/carrier.rs`.
