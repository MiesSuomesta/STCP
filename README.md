# STCP
Secure TCP protocol with AES security layer.

# STCP packet format

Packet format over standard TCP packet-payload: [ 16/24/32 bytes of AES IV key ] + [ the AES-encrypted payload ]

# STCP Packet handling

Incoming: fetch the IV-vector of 16/24/32 bytes from incoming packet -> use it and predefined AES key to decrypt package, prior to handing the TCP-packet to message handler.

Outgoing: Generate random IV-vector of 16/24/32 bytes and apply to outgoint TCP packet -> use it and predefined AES key to encrypt package, prior to handing the TCP-packet to sending the message.

# Rationale: Why STCP is Better Than TLS

Traditional TLS (Transport Layer Security) provides encryption on top of TCP, but it requires significant manual setup — certificates, key management, configuration, and often separate libraries or integrations per application.

STCP (Secure TCP) eliminates all that complexity by providing fully automatic encryption with zero configuration. 

# Zero Configuration, Maximum Security

STCP provides end-to-end encryption automatically.
No certificates, no configuration files, no setup steps.

# Fully Automatic Key Management

TLS requires manually issued X.509 certificates and trusted CAs.
STCP manages all keys dynamically and ephemerally:

Each connection creates its own one-time encryption key.

Keys are exchanged securely without any third-party authority.

Keys are destroyed immediately when the connection ends.

This means no key renewal, no certificate expiration, and no leaks to worry about.

# No Dependence on Certificate Authorities

TLS relies on external Certificate Authorities (CAs) for trust.
If a CA is compromised, the entire security model collapses.

STCP is self-contained and trustless — the two endpoints negotiate their encryption keys directly.
No third parties, no external dependencies, no fake certificates.

# Per-Message Encryption for Total Privacy

While TLS typically uses one session key per connection, STCP goes further:
every single message is encrypted with its own unique key.

This makes passive traffic analysis and replay attacks practically impossible.
Even if one message were somehow compromised, others remain completely secure.

# No Configuration Errors

Most real-world security issues stem from misconfiguration, not protocol flaws — weak cipher suites, expired certificates, or missing hostname verification.
STCP eliminates that entire class of problems.
There is simply nothing to misconfigure.

# Built-In Protection Against MITM Attacks

Because STCP uses ephemeral keys and authenticated encryption for each exchange, Man-In-The-Middle attacks are mathematically infeasible.
If the handshake is intercepted, the connection fails silently — not insecurely.
