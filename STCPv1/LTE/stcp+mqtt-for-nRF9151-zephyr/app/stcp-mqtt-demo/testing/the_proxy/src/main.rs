mod bridge;
mod cleanup;
mod debug;
mod handshake;
mod mqtt;
mod proxy;
mod transport;
mod types;

use proxy::run_proxy_server;

fn main() {
    run_proxy_server("0.0.0.0:7777");
}
