# STCP native P2P Zephyr application

This project is the P2P/Noise application split out of the normal STCP
transport regression application. It intentionally keeps the existing P2P
wire behavior and command structure; the refactor only moves ownership.

Build:

    ./scripts/build.sh
    ./scripts/flash.sh

Shell:

    stcp p2p show
    stcp p2p noise
    stcp p2p ping 4
    stcp p2p bench upload
    stcp p2p bench download
    stcp p2p bench full
    stcp p2p bench all

Default peer is 192.168.1.20:19010 and can be changed at runtime with
`stcp p2p host` and `stcp p2p port`.

The regular `application/` no longer compiles, configures or registers P2P.
Noise and native P2P source now live only in this project.
