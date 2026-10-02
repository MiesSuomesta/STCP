use stcp_mini_sdk::{parse_ipv4, StcpStream};

fn main() -> std::io::Result<()> {
    let remote = std::env::args().nth(1).unwrap_or_else(|| "127.0.0.1:19002".into());
    let message = std::env::args().nth(2).unwrap_or_else(|| "Hello STCP".into());

    let mut stream = StcpStream::connect(parse_ipv4(&remote)?)?;
    stream.send_all(message.as_bytes())?;

    let mut reply = vec![0u8; message.len()];
    let mut got = 0;
    while got < reply.len() {
        let n = stream.recv(&mut reply[got..])?;
        if n == 0 { break; }
        got += n;
    }
    reply.truncate(got);

    if reply != message.as_bytes() {
        return Err(std::io::Error::new(std::io::ErrorKind::InvalidData, "echo mismatch"));
    }
    println!("PASS: {} bytes: {}", got, String::from_utf8_lossy(&reply));
    Ok(())
}
