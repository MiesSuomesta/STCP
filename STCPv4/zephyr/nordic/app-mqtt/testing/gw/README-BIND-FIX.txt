STCPv2 MQTT gateway bind fix

Fix:
- socket() remains: AF_STCP / SOCK_STREAM / protocol 253
- bind() uses an IPv4 sockaddr_in with sin_family = AF_INET

Before:
    addr.sin_family = AF_STCP

After:
    addr.sin_family = AF_INET

Reason:
AF_STCP selects the custom socket provider. The address passed to bind/connect
is still an IPv4 sockaddr_in and therefore carries AF_INET in sin_family.

No other gateway behavior changed.
