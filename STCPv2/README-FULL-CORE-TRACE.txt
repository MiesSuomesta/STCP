STCPv2 canonical-core full trace overlay

This overlay adds a broad debug_event() trace net to the Rust core so future
Zephyr/Linux/RPi debugging does not require adding one-off markers repeatedly.

Event families:
  260-279  handshake internals
  320-329  connection / handshake entrypoints
  330-339  send / recv / accept / release / tick
  340-349  carrier ingress
  350-359  frame extraction / decode

Important handshake events:
  260 process_handshake_frames enter
  261 PublicKey received
  262 derive_session_keys start
  263 derive_session_keys done
  264 HandshakeDone encode start
  265 HandshakeDone encode done (arg0 = frame length)
  266 HandshakeDone send start
  267 HandshakeDone send done
  268 peer HandshakeDone received
  269 ready-state evaluation (arg0 = crypto.ready, arg1 = peer_done)
  270 session/socket becomes READY
  271 process_handshake_frames exit
  272 decoded handshake frame observed
  273 shared-secret derivation path reached

Major operation events:
  320 connect enter
  321 start_handshake enter
  330 send enter
  331 recv enter
  332 accept enter
  333 create_external_tcp_child enter
  334 release enter
  335 tick enter
  336 application frame encode
  337 application frame send
  340 carrier ingress enter
  341 carrier ingress exit
  350 frame extraction/decode enter

The existing C/Zephyr platform stcp_kernel_debug_event() hook remains the
single logging backend. Linux/RPi builds keep using the same canonical core.

Install at repository root and rebuild.
