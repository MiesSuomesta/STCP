use stcp_mini_sdk::{any, parse_ipv4, StcpListener};

fn main() -> std::io::Result<()> {
    let bind = std::env::args().nth(1)
        .map(|s| parse_ipv4(&s))
        .transpose()?
        .unwrap_or_else(|| any(19002));

    let listener = StcpListener::bind(bind)?;
    println!("STCP echo server listening on {bind}");

    loop {
        let mut stream = listener.accept()?;
        std::thread::spawn(move || -> std::io::Result<()> {
            let mut buf = [0u8; 64 * 1024];
            loop {
                let n = stream.recv(&mut buf)?;
                if n == 0 { return Ok(()); }
                stream.send_all(&buf[..n])?;
            }
        });
    }
}
