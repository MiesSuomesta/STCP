# STCPv2 socket skeleton

Puhdas socket-pohjainen STCPv2-runko.

Ei `TcpStream`-abstraktiota STCP-coressa, ei tick-looppeja, ei FSM-event-bussia.
Core käyttää raw fd:tä ja libc:n `socket/bind/listen/accept/connect/send/recv/close`-funktioita.

## Wire protocol

Kaikki STCP-paketit alkavat 16 tavun headerilla:

```text
[4B magic   = "STCP"]
[1B type]
[1B version = 1]
[2B flags]
[8B payload_len]
[payload...]
```

Packet types:

```text
1 = PublicKey
2 = Data
3 = Close
4 = Error
```

Payloadit:

```text
PublicKey: 64B public key
Data:      16B IV + ciphertext
Close:     reason bytes
Error:     error bytes
```

Huom: nykyinen crypto-runko käyttää X25519:ää, jossa oikea public key on 32B. Wire-public-key on silti 64B; jälkimmäiset 32B ovat nyt reserved/nollia. Jos vanha `crypto.rs` tuottaa oikean 64B public keyn, korvaa `crypto.rs` sisuskalut sillä.

## API-ajatus

```rust
stcp_socket()
stcp_bind(state, addr)
stcp_listen(state, backlog)
stcp_accept(&state)
stcp_connect(state, addr)
stcp_send(&mut state, data)
stcp_recv(&mut state, out)
stcp_close(state, reason)
```

State kantaa ctx:n mukana:

```rust
pub enum StcpState {
    New(StcpContext),
    Bound { ctx, at_where },
    Listening { ctx, at_where },
    Connected { ctx, to_where },
    Ready { ctx },
    Closing { ctx, reason },
    Closed { ctx, reason },
    Error { ctx, reason },
}
```

## Testi

Terminal 1:

```bash
cargo run -p stcp-runner -- server 127.0.0.1:7777
```

Terminal 2:

```bash
cargo run -p stcp-runner -- client 127.0.0.1:7777 "moro"
cargo run -p stcp-runner -- stress 127.0.0.1:7777 100
```
