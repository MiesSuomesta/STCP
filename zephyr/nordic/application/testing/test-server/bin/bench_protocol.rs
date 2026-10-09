//! BEN2 benchmark protocol, operating on decrypted STCP application bytes.
const MAGIC: u32 = 0x4245_4e32;
const VERSION: u32 = 1;
const MAX_TOTAL: u32 = 256 * 1024 * 1024;

fn word(buf: &[u8], at: usize) -> u32 {
    u32::from_be_bytes(buf[at..at + 4].try_into().unwrap())
}
fn pattern(offset: u32, salt: u8) -> u8 {
    (offset.wrapping_mul(31).wrapping_add(salt as u32) & 255) as u8
}

#[derive(Clone, Copy, Debug)]
struct Request { mode: u32, chunk: u32, total: u32 }

#[derive(Debug)]
pub enum Protocol {
    Detect(Vec<u8>),
    Echo,
    Bench(Bench),
}

#[derive(Debug)]
pub struct Bench {
    request: Request,
    received: u32,
    sent: u32,
    ack: Vec<u8>,
    completed: bool,
}

impl Protocol {
    pub fn new() -> Self { Self::Detect(Vec::new()) }

    pub fn on_plain(&mut self, bytes: &[u8], send: &mut impl FnMut(&[u8]) -> Result<(), String>) -> Result<(), String> {
        match self {
            Self::Echo => send(bytes),
            Self::Detect(pending) => {
                pending.extend_from_slice(bytes);
                if pending.len() >= 4 && pending[..4] != MAGIC.to_be_bytes() {
                    let echo = std::mem::take(pending);
                    *self = Self::Echo;
                    return send(&echo);
                }
                if pending.len() < 20 { return Ok(()); }
                let data = std::mem::take(pending);
                let version = word(&data, 4);
                let request = Request { mode: word(&data, 8), chunk: word(&data, 12), total: word(&data, 16) };
                if version != VERSION || !(1..=3).contains(&request.mode) || request.chunk == 0 || request.total == 0 || request.total > MAX_TOTAL {
                    return Err(format!("invalid BEN2 request version={version} request={request:?}"));
                }
                eprintln!("[BEN2] start mode={} chunk={} total={}", request.mode, request.chunk, request.total);
                *self = Self::Bench(Bench { request, received: 0, sent: 0, ack: Vec::new(), completed: false });
                // Download is pumped incrementally by the connection loop.
                self.on_plain(&data[20..], send)
            }
            Self::Bench(bench) => {
                if bench.completed { return Err("BEN2 data after completed request".into()); }
                let mut pos = 0;
                if bench.request.mode != 2 {
                    let remaining = (bench.request.total - bench.received) as usize;
                    let count = remaining.min(bytes.len());
                    for (i, &byte) in bytes[..count].iter().enumerate() {
                        if byte != pattern(bench.received + i as u32, 0x17) {
                            return Err(format!("BEN2 payload mismatch offset={}", bench.received + i as u32));
                        }
                    }
                    bench.received += count as u32;
                    pos = count;
                }
                if bench.request.mode == 1 {
                    if bench.received == bench.request.total {
                        bench.completed = true;
                        send(&reply(1, 0, bench.received, 0))?;
                        eprintln!("[BEN2] upload PASS bytes={}", bench.received);
                    }
                    if pos != bytes.len() { return Err("BEN2 trailing upload bytes".into()); }
                    return Ok(());
                }
                bench.ack.extend_from_slice(&bytes[pos..]);
                if bench.ack.len() > 20 { return Err("BEN2 trailing ACK bytes".into()); }
                if bench.ack.len() == 20 && bench.sent == bench.request.total && bench.received == if bench.request.mode == 3 { bench.request.total } else { 0 } {
                    if word(&bench.ack, 0) != MAGIC || word(&bench.ack, 4) != bench.request.mode || word(&bench.ack, 8) != 0 || word(&bench.ack, 12) != bench.request.total || word(&bench.ack, 16) != if bench.request.mode == 3 { bench.request.total } else { 0 } {
                        return Err("BEN2 invalid client ACK".into());
                    }
                    bench.completed = true;
                    send(&reply(bench.request.mode, 0, bench.received, bench.sent))?;
                    eprintln!("[BEN2] mode={} PASS rx={} tx={}", bench.request.mode, bench.received, bench.sent);
                }
                Ok(())
            }
        }
    }

    pub fn pump_download(&mut self, send: &mut impl FnMut(&[u8]) -> Result<(), String>) -> Result<(), String> {
        let Self::Bench(bench) = self else { return Ok(()); };
        if bench.request.mode == 1 || bench.sent >= bench.request.total { return Ok(()); }
        // Keep each STCP write bounded. In FULL mode, the receive side must
        // continue making progress instead of sending the entire download first.
        let count = (bench.request.chunk as usize).min(1024)
            .min((bench.request.total - bench.sent) as usize);
        let mut buffer = vec![0u8; count];
        for (i, byte) in buffer.iter_mut().enumerate() {
            *byte = pattern(bench.sent + i as u32, 0x53);
        }
        send(&buffer)?;
        bench.sent += count as u32;
        if bench.sent == bench.request.total {
            eprintln!("[BEN2] download queued mode={} bytes={}", bench.request.mode, bench.sent);
        }
        Ok(())
    }

}

fn reply(mode: u32, status: u32, received: u32, sent: u32) -> [u8; 20] {
    let mut out = [0u8; 20];
    for (i, val) in [MAGIC, mode, status, received, sent].iter().enumerate() {
        out[i * 4..i * 4 + 4].copy_from_slice(&val.to_be_bytes());
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn upload() {
        let mut protocol = Protocol::new();
        let mut sent = Vec::new();
        let mut request = Vec::new();
        for word in [MAGIC, VERSION, 1, 4, 9] { request.extend_from_slice(&word.to_be_bytes()); }
        protocol.on_plain(&request[..8], &mut |b| { sent.extend_from_slice(b); Ok(()) }).unwrap();
        protocol.on_plain(&request[8..], &mut |b| { sent.extend_from_slice(b); Ok(()) }).unwrap();
        let data: Vec<_> = (0..9).map(|i| pattern(i, 0x17)).collect();
        protocol.on_plain(&data, &mut |b| { sent.extend_from_slice(b); Ok(()) }).unwrap();
        assert_eq!(sent, reply(1, 0, 9, 0));
    }
}

#[cfg(test)]
mod integration_tests {
    use super::*;
    fn req(mode: u32, chunk: u32, total: u32) -> Vec<u8> {
        [MAGIC, VERSION, mode, chunk, total].iter().flat_map(|w| w.to_be_bytes()).collect()
    }
    #[test]
    fn download_ack() {
        let mut p = Protocol::new();
        let mut out = Vec::new();
        p.on_plain(&req(2, 4096, 4096), &mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        assert!(out.is_empty());
        for _ in 0..4 { p.pump_download(&mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap(); }
        assert_eq!(out.len(), 4096);
        assert!(out.iter().enumerate().all(|(i,b)| *b == pattern(i as u32, 0x53)));
        p.on_plain(&reply(2, 0, 4096, 0), &mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        assert_eq!(&out[4096..], &reply(2, 0, 0, 4096));
    }
    #[test]
    fn full_duplex_interleaved() {
        let mut p = Protocol::new();
        let mut out = Vec::new();
        p.on_plain(&req(3, 4096, 2048), &mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        p.pump_download(&mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        let data: Vec<_> = (0..2048).map(|i| pattern(i, 0x17)).collect();
        p.on_plain(&data[..1024], &mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        p.pump_download(&mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        p.on_plain(&data[1024..], &mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        p.on_plain(&reply(3, 0, 2048, 2048), &mut |b| { out.extend_from_slice(b); Ok(()) }).unwrap();
        assert_eq!(out.len(), 2068);
        assert_eq!(&out[2048..], &reply(3, 0, 2048, 2048));
    }
}
