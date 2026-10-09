# STCPv4 AES / P2P crypto boundary

Required data path:

    P2P/libp2p plaintext -> STCP-TCP -> AES-256-GCM -> TCP carrier -> wire
    wire -> TCP carrier -> AES-256-GCM -> STCP-TCP -> P2P/libp2p plaintext

Changes in this package:

- STCP `CryptoContext` remains AES-256-GCM only.
- P2P Noise ChaChaPoly helpers are explicitly named `stcp_p2p_noise_chacha_*`.
- Removed the unused generic ChaCha in-place decrypt helper.
- Corrected stale ChaCha comments in the STCP AES send/RX path.
- Documented that libp2p Noise/ChaChaPoly is an inner P2P protocol only; it is
  not an STCP crypto mode, fallback, or carrier bypass.

Important: libp2p Noise XX still requires its specified ChaChaPoly primitive for
interoperability. Those bytes are then carried as P2P data through STCP-TCP,
which applies STCP AES-256-GCM before the TCP carrier.
