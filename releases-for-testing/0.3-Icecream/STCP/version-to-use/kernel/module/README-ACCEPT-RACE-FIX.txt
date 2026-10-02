STCP accepted-handshake race fix
================================

Fixes fast same-host STCP-TCP accept where the carrier RX worker advances the
accepted Rust child handshake before stcp_accept() calls
stcp_rust_start_handshake().

Only -EINVAL with a non-zero connection id is treated as the benign
"already advanced by RX" race. All other handshake-start errors remain fatal.

Extract in the kernel/module directory that contains src/stcp_ops.c.
