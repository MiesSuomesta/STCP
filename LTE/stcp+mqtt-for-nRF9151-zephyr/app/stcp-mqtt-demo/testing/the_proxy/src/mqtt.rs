use std::net::TcpStream;

pub fn connect_to_broker(
    addr: &str
) -> TcpStream {

    TcpStream::connect(addr)
        .expect("MQTT connect failed")
}
