//! Linux BEN2 native socket server. Standard library only; STCP uses stcp.ko.
mod bench_protocol;
use std::{collections::VecDeque, io, mem, os::fd::{AsRawFd, FromRawFd, OwnedFd}, time::{Duration,Instant}};

const AF_INET:i32=2;
const AF_STCP:i32=45;
const MSG_DONTWAIT:i32=0x40;
const MSG_NOSIGNAL:i32=0x4000;
const BUD2:u32=0x42554432;
#[repr(C)]
#[derive(Clone,Copy,Default)]
struct Address {family:u16,port:u16,addr:u32,zero:[u8;8]}
#[repr(C)]
struct PollFd {fd:i32,events:i16,revents:i16}
#[repr(C)]
struct Timeval {sec:std::ffi::c_long,usec:std::ffi::c_long}
unsafe extern "C" {
    fn socket(domain:i32,kind:i32,protocol:i32)->i32;
    fn bind(fd:i32,address:*const Address,len:u32)->i32;
    fn listen(fd:i32,backlog:i32)->i32;
    fn accept(fd:i32,address:*mut Address,len:*mut u32)->i32;
    fn setsockopt(fd:i32,level:i32,name:i32,value:*const std::ffi::c_void,len:u32)->i32;
    fn recv(fd:i32,data:*mut u8,len:usize,flags:i32)->isize;
    fn send(fd:i32,data:*const u8,len:usize,flags:i32)->isize;
    fn recvfrom(fd:i32,data:*mut u8,len:usize,flags:i32,address:*mut Address,alen:*mut u32)->isize;
    fn sendto(fd:i32,data:*const u8,len:usize,flags:i32,address:*const Address,alen:u32)->isize;
    fn poll(fds:*mut PollFd,count:usize,timeout:i32)->i32;
}
fn error()->String {io::Error::last_os_error().to_string()}
fn checked(rc:i32)->Result<(),String> {if rc<0 {Err(error())}else{Ok(())}}
fn owned(fd:i32)->Result<OwnedFd,String> {if fd<0 {Err(error())}else{Ok(unsafe{OwnedFd::from_raw_fd(fd)})}}
fn word(b:&[u8],offset:usize)->u32 {u32::from_be_bytes(b[offset..offset+4].try_into().unwrap())}
struct Socket {
    fd:OwnedFd, plain_udp:bool, framed:bool, peer:Option<Address>, run:Option<u32>,
    tx_seq:u32,rx_seq:u32,pending:Vec<u8>,
    outgoing:VecDeque<Vec<u8>>, outgoing_offset:usize, queued_bytes:usize,
    udp_gap:Duration,next_udp_send:Instant,
}
impl Socket {
    fn new(fd:OwnedFd,transport:&str,timeout_ms:u64)->Result<Self,String> {
        let raw=fd.as_raw_fd();
        let tv=Timeval{sec:(timeout_ms/1000)as _,usec:((timeout_ms%1000)*1000)as _};
        checked(unsafe{setsockopt(raw,1,21,(&tv as *const Timeval).cast(),mem::size_of::<Timeval>()as u32)})?;
        Ok(Self{fd,plain_udp:transport=="udp",framed:transport.ends_with("udp"),peer:None,run:None,tx_seq:0,rx_seq:0,pending:Vec::with_capacity(2081),outgoing:VecDeque::new(),outgoing_offset:0,queued_bytes:0,udp_gap:Duration::ZERO,next_udp_send:Instant::now()})
    }
    fn receive(&mut self,buffer:&mut [u8])->Result<Option<usize>,String> {
        if self.framed && !self.plain_udp {
            if let Some(n)=self.extract(buffer)? {return Ok(Some(n));}
        }
        let mut raw=[0u8;16384];let capacity=if self.framed {1041}else{raw.len()};
        let mut peer=Address::default();let mut size=mem::size_of::<Address>()as u32;
        let n=unsafe{if self.plain_udp {recvfrom(self.fd.as_raw_fd(),raw.as_mut_ptr(),capacity,MSG_DONTWAIT,&mut peer,&mut size)}
            else {recv(self.fd.as_raw_fd(),raw.as_mut_ptr(),capacity,MSG_DONTWAIT)}};
        if n<0 {
            let e=io::Error::last_os_error();
            if e.kind()==io::ErrorKind::WouldBlock || e.kind()==io::ErrorKind::Interrupted {return Ok(None);}
            return Err(e.to_string());
        }
        if n==0 && !self.plain_udp {return Err("peer closed".into());}
        let bytes=&raw[..n as usize];
        if !self.framed {buffer[..bytes.len()].copy_from_slice(bytes);return Ok(Some(bytes.len()));}
        if self.plain_udp {
            if let Some(p)=self.peer {if p.addr!=peer.addr || p.port!=peer.port {return Ok(None);}}
            if self.run.is_none() {
                // Ignore stale/non-request packets while the UDP listener is idle.
                if bytes.len()!=36 || word(bytes,0)!=BUD2 || word(bytes,8)!=0 || word(bytes,12)!=20 || word(bytes,16)!=0x42454e32 {return Ok(None);}
            }else if bytes.len()>=16 && Some(word(bytes,4))!=self.run {return Ok(None);}
            if bytes.len()<16 {return Err("short UDP envelope".into());}
            let length=word(bytes,12)as usize;
            if length==0 || length>1024 || bytes.len()!=length+16 {return Err("invalid UDP envelope length".into());}
            self.pending.extend_from_slice(bytes);
            let result=self.extract(buffer)?;
            if !self.pending.is_empty() {return Err("UDP envelope trailing data".into());}
            if self.peer.is_none() {self.peer=Some(peer);}
            return Ok(result);
        }
        if self.pending.len()+bytes.len()>2081 {return Err("STCP-UDP framing overflow".into());}
        self.pending.extend_from_slice(bytes);self.extract(buffer)
    }
    fn extract(&mut self,buffer:&mut [u8])->Result<Option<usize>,String> {
        if self.pending.len()<16 {return Ok(None);}
        let magic=word(&self.pending,0);let run=word(&self.pending,4);
        let seq=word(&self.pending,8);let length=word(&self.pending,12)as usize;
        if magic!=BUD2 || length==0 || length>1024 || length>buffer.len() || seq!=self.rx_seq || self.run.is_some_and(|r|r!=run) {return Err("invalid UDP run/sequence/length".into());}
        if self.run.is_none() && (seq!=0 || length!=20) {return Err("missing UDP BEN2 request".into());}
        if self.pending.len()<length+16 {return Ok(None);}
        buffer[..length].copy_from_slice(&self.pending[16..16+length]);
        self.pending.drain(..length+16);self.run=Some(run);
        self.rx_seq=self.rx_seq.checked_add(1).ok_or("UDP sequence overflow")?;
        Ok(Some(length))
    }
    fn transmit(&mut self,data:&[u8])->Result<(),String> {
        let mut envelope=[0u8;1040];
        let bytes=if self.framed {
            if data.is_empty() || data.len()>1024 {return Err("UDP payload too large".into());}
            let run=self.run.ok_or("UDP send before request")?;
            for(i,v)in [BUD2,run,self.tx_seq,data.len()as u32].iter().enumerate(){envelope[i*4..i*4+4].copy_from_slice(&v.to_be_bytes());}
            envelope[16..16+data.len()].copy_from_slice(data);&envelope[..16+data.len()]
        } else {data};
        // Queue one complete envelope. A partial stream write resumes from
        // its byte offset; native UDP remains one atomic datagram.
        if self.queued_bytes + bytes.len() > 65536 {return Err("BEN2 TX queue overflow".into());}
        let next_seq=if self.framed {self.tx_seq.checked_add(1).ok_or("UDP sequence overflow")?}else{self.tx_seq};
        self.outgoing.push_back(bytes.to_vec());self.queued_bytes+=bytes.len();
        self.tx_seq=next_seq;
        Ok(())
    }
    fn flush(&mut self)->Result<bool,String> {
        let Some(bytes)=self.outgoing.front() else {return Ok(false);};
        if self.plain_udp && Instant::now()<self.next_udp_send {return Ok(false);}
        let offset=self.outgoing_offset;
        let n=unsafe{if self.plain_udp {
            let peer=self.peer.ok_or("UDP send before peer")?;
            sendto(self.fd.as_raw_fd(),bytes.as_ptr(),bytes.len(),MSG_NOSIGNAL|MSG_DONTWAIT,&peer,mem::size_of::<Address>()as u32)
        }else{send(self.fd.as_raw_fd(),bytes[offset..].as_ptr(),bytes.len()-offset,MSG_NOSIGNAL|MSG_DONTWAIT)}};
        if n<0 {
            let e=io::Error::last_os_error();
            if e.kind()==io::ErrorKind::WouldBlock || e.kind()==io::ErrorKind::Interrupted {return Ok(false);}
            return Err(e.to_string());
        }
        if n==0 {return Err("zero-length socket send".into());}
        let n=n as usize;
        if n>bytes.len()-offset || (self.plain_udp && n!=bytes.len()) {return Err("invalid partial socket send".into());}
        if self.plain_udp {self.next_udp_send=Instant::now()+self.udp_gap;}
        self.outgoing_offset+=n;self.queued_bytes-=n;
        if self.outgoing_offset==bytes.len() {self.outgoing.pop_front();self.outgoing_offset=0;}
        Ok(true)
    }
    fn wait(&self,ms:i32)->Result<(),String> {
        let mut p=PollFd{fd:self.fd.as_raw_fd(),events:1,revents:0};
        let rc=unsafe{poll(&mut p,1,ms)};
        if rc<0 && io::Error::last_os_error().kind()!=io::ErrorKind::Interrupted {return Err(error());}Ok(())
    }
    fn reset_udp(&mut self) {self.peer=None;self.run=None;self.tx_seq=0;self.rx_seq=0;self.pending.clear();self.outgoing.clear();self.outgoing_offset=0;self.queued_bytes=0;self.next_udp_send=Instant::now();}
}
fn run(socket:&mut Socket,timeout:Duration)->Result<(),String> {
    let mut protocol=bench_protocol::Protocol::new();let mut buffer=[0u8;16384];
    let mut last=Instant::now();let mut begun=!socket.plain_udp; let mut first_data=true;
    let result=(|| { loop {
        let mut progressed=socket.flush()?;
        // Reserve room for a maximum stream echo and its completion reply.
        // Always receive during download backpressure; do not block in send.
        if socket.queued_bytes <= 32768 {
            match socket.receive(&mut buffer) {
                Ok(Some(n))=>{
                    if first_data {eprintln!("BEN2_RX_FIRST bytes={n} prefix={:02x?}", &buffer[..n.min(20)]); first_data=false;}
                    begun=true;progressed=true;
                    protocol.on_plain(&buffer[..n],&mut |b|socket.transmit(b))?;}
                Ok(None)=>{},Err(e)=>return Err(e),
            }
        }
        if socket.outgoing.is_empty() {
            protocol.pump_download(&mut |b|socket.transmit(b))?;
        }
        progressed |= socket.flush()?;
        if progressed {last=Instant::now();}
        protocol.progress();
        if protocol.done() && socket.outgoing.is_empty() {return Ok(());}
        if begun && last.elapsed()>=timeout {return Err("BEN2 inactivity timeout".into());}
        if !progressed {
            // POLLOUT on STCP can be optimistic. A bounded sleep avoids a
            // busy loop while still servicing both directions every millisecond.
            if socket.outgoing.is_empty() {socket.wait(20)?;}
            else {std::thread::sleep(Duration::from_millis(1));}
        }
    } })();
    if result.is_err() {protocol.fail();}
    result
}
fn usage(){eprintln!("ben2-server --transport all|tcp|udp|stcp-tcp|stcp-udp [--bind IPv4] [--port 19000] [--timeout-ms 60000] [--report-ms 5000] [--udp-gap-us 10000]\n--udp-gap-us sets native UDP TX spacing; 0 disables pacing. STCP is unaffected. --report-ms 0 disables interim reports. STCP modes require stcp.ko.");}
fn main()->Result<(),String> {
    if !cfg!(target_os="linux") {return Err("native STCP server requires Linux".into());}
    let mut transport="stcp-tcp".to_string();let mut address="0.0.0.0".to_string();
    let mut port=19000u16;let mut timeout=60000u64;let mut report=5000u64;let mut udp_gap=10000u64;
    let mut args=std::env::args().skip(1);
    while let Some(arg)=args.next(){if arg=="--help" {usage();return Ok(());}
        let value=args.next().ok_or_else(||format!("missing value for {arg}"))?;
        match arg.as_str(){"--transport"=>transport=value,"--bind"=>address=value,
            "--port"=>port=value.parse().map_err(|_|"bad port")?,"--timeout-ms"=>timeout=value.parse().map_err(|_|"bad timeout")?,
            "--report-ms"=>report=value.parse().map_err(|_|"bad report interval")?,"--udp-gap-us"=>udp_gap=value.parse().map_err(|_|"bad UDP gap")?,_=>return Err(format!("unknown option {arg}"))}}
    if port==0 || timeout==0 || timeout>86_400_000 || report>3_600_000 || udp_gap>1_000_000 {return Err("invalid port/timeout/report interval".into());}
    if transport=="all" {
        if port>65532 {return Err("matrix base port must be <=65532".into());}
        let executable=std::env::current_exe().map_err(|e|e.to_string())?;
        let mut children:Vec<std::process::Child>=Vec::new();
        for(i,t)in ["stcp-tcp","stcp-udp","tcp","udp"].iter().enumerate() {
            let spawned=std::process::Command::new(&executable).args([
                "--transport",t,"--bind",&address,"--port",&(port+i as u16).to_string(),
                "--timeout-ms",&timeout.to_string(),"--report-ms",&report.to_string(),"--udp-gap-us",&udp_gap.to_string()]).spawn();
            match spawned {Ok(child)=>children.push(child),Err(e)=>{for child in &mut children {let _=child.kill();let _=child.wait();}return Err(e.to_string());}}
        }
        loop {
            let mut ended=None;
            for child in &mut children {
                match child.try_wait() {Ok(Some(status))=>{ended=Some(format!("matrix listener exited: {status}"));break;},Ok(None)=>{},Err(e)=>{ended=Some(e.to_string());break;}}
            }
            if let Some(error)=ended {for child in &mut children {let _=child.kill();let _=child.wait();}return Err(error);}
            std::thread::sleep(Duration::from_millis(100));
        }
    }
    let (family,kind,proto)=match transport.as_str(){"tcp"=>(AF_INET,1,6),"udp"=>(AF_INET,2,17),"stcp-tcp"=>(AF_STCP,1,253),"stcp-udp"=>(AF_STCP,1,254),_=>return Err("invalid transport".into())};
    // Single-threaded server: configured before processing any request.
    unsafe{std::env::set_var("BEN2_TRANSPORT",&transport);std::env::set_var("BEN2_REPORT_MS",report.to_string());}
    let ip:std::net::Ipv4Addr=address.parse().map_err(|_|"invalid IPv4 bind")?;
    let a=Address{family:AF_INET as u16,port:port.to_be(),addr:u32::from_ne_bytes(ip.octets()),zero:[0;8]};
    let fd=owned(unsafe{socket(family,kind,proto)})?;let raw=fd.as_raw_fd();
    let one=1i32;let queue=4*1024*1024i32;
    for(name,value)in [(2,one),(8,queue),(7,queue)] {unsafe{setsockopt(raw,1,name,(&value as *const i32).cast(),4)};}
    checked(unsafe{bind(raw,&a,mem::size_of::<Address>()as u32)})?;
    if transport!="udp" {checked(unsafe{listen(raw,16)})?;}
    eprintln!("BENCH_SERVER_READY transport={transport} bind={address} port={port} report_ms={report} udp_gap_us={udp_gap}");
    if transport=="udp" {
        let mut socket=Socket::new(fd,&transport,timeout)?;
        socket.udp_gap=Duration::from_micros(udp_gap);
        loop {if let Err(e)=run(&mut socket,Duration::from_millis(timeout)){eprintln!("BEN2_ERROR {e}");}socket.reset_udp();}
    }else{
        loop {
            let accepted=unsafe{accept(raw,std::ptr::null_mut(),std::ptr::null_mut())};
            if accepted<0 {let e=io::Error::last_os_error();if e.kind()==io::ErrorKind::Interrupted || e.kind()==io::ErrorKind::WouldBlock || e.kind()==io::ErrorKind::TimedOut {continue;}return Err(e.to_string());}
            eprintln!("BEN2_ACCEPT transport={transport} fd={accepted}");
            let mut socket=Socket::new(owned(accepted)?,&transport,timeout)?;
            if let Err(e)=run(&mut socket,Duration::from_millis(timeout)){eprintln!("BEN2_ERROR {e}");}
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::os::unix::net::UnixStream;
    fn framed()->Socket {
        let(a,_b)=UnixStream::pair().unwrap();
        Socket::new(a.into(),"stcp-udp",1000).unwrap()
    }
    fn envelope(run:u32,seq:u32,data:&[u8])->Vec<u8> {
        let mut out=Vec::new();
        for v in [BUD2,run,seq,data.len()as u32] {out.extend_from_slice(&v.to_be_bytes());}
        out.extend_from_slice(data);out
    }
    #[test]
    fn split_header_and_payload_are_retained() {
        let mut s=framed();let packet=envelope(7,0,&[0;20]);let mut out=[0;1024];
        s.pending.extend_from_slice(&packet[..7]);assert_eq!(s.extract(&mut out).unwrap(),None);
        s.pending.extend_from_slice(&packet[7..18]);assert_eq!(s.extract(&mut out).unwrap(),None);
        s.pending.extend_from_slice(&packet[18..]);assert_eq!(s.extract(&mut out).unwrap(),Some(20));
        assert!(s.pending.is_empty());assert_eq!(s.rx_seq,1);
    }
    #[test]
    fn coalesced_envelopes_are_separate() {
        let mut s=framed();let mut out=[0;1024];
        s.pending.extend(envelope(7,0,&[0;20]));s.pending.extend(envelope(7,1,&[3;1024]));
        assert_eq!(s.extract(&mut out).unwrap(),Some(20));
        assert_eq!(s.extract(&mut out).unwrap(),Some(1024));assert_eq!(out,[3;1024]);
    }
    #[test]
    fn missing_or_duplicate_sequence_is_rejected() {
        for seq in [0,2] {
            let mut s=framed();let mut out=[0;1024];s.pending.extend(envelope(7,0,&[0;20]));s.extract(&mut out).unwrap();
            s.pending.extend(envelope(7,seq,&[0;1024]));assert!(s.extract(&mut out).is_err());
        }
    }
    #[test]
    fn oversize_and_wrong_run_are_rejected() {
        let mut out=[0;2048];let mut s=framed();s.pending.extend(envelope(7,0,&[0;1025]));assert!(s.extract(&mut out).is_err());
        let mut s=framed();s.pending.extend(envelope(7,0,&[0;20]));s.extract(&mut out).unwrap();
        s.pending.extend(envelope(8,1,&[0;10]));assert!(s.extract(&mut out).is_err());
    }
    #[test]
    fn queued_envelopes_resume_a_partial_stream_write() {
        let(a,mut b)=UnixStream::pair().unwrap();
        let mut s=Socket::new(a.into(),"stcp-udp",1000).unwrap();s.run=Some(7);
        s.transmit(&[3;1024]).unwrap();s.transmit(&[4;20]).unwrap();
        let expected=[envelope(7,0,&[3;1024]),envelope(7,1,&[4;20])].concat();
        // The OS has accepted only this prefix of the first envelope.
        assert_eq!(unsafe{send(s.fd.as_raw_fd(),expected.as_ptr(),7,MSG_NOSIGNAL|MSG_DONTWAIT)},7);
        s.outgoing_offset=7;s.queued_bytes-=7;
        while !s.outgoing.is_empty() {assert!(s.flush().unwrap());}
        assert_eq!(s.queued_bytes,0);assert_eq!(s.tx_seq,2);
        use std::io::Read;
        let mut received=vec![0;expected.len()];b.read_exact(&mut received).unwrap();assert_eq!(received,expected);
    }
    #[test]
    fn send_backpressure_returns_without_blocking() {
        let(a,b)=UnixStream::pair().unwrap();
        let mut s=Socket::new(a.into(),"stcp-udp",1000).unwrap();s.run=Some(7);
        let fill=[0u8;4096];
        loop {
            let n=unsafe{send(s.fd.as_raw_fd(),fill.as_ptr(),fill.len(),MSG_NOSIGNAL|MSG_DONTWAIT)};
            if n<0 {assert_eq!(io::Error::last_os_error().kind(),io::ErrorKind::WouldBlock);break;}
            assert!(n>0);
        }
        s.transmit(&[3;1024]).unwrap();
        assert!(!s.flush().unwrap());assert_eq!(s.outgoing_offset,0);assert_eq!(s.queued_bytes,1040);
        // RX still works while the TX side is full.
        let packet=envelope(7,0,&[0;20]);
        assert_eq!(unsafe{send(b.as_raw_fd(),packet.as_ptr(),packet.len(),MSG_NOSIGNAL|MSG_DONTWAIT)},packet.len()as isize);
        let mut out=[0;1024];assert_eq!(s.receive(&mut out).unwrap(),Some(20));
    }

    #[test]
    fn udp_pacing_retains_the_unsent_datagram() {
        let(a,_b)=UnixStream::pair().unwrap();
        let mut s=Socket::new(a.into(),"stcp-udp",1000).unwrap();s.run=Some(7);
        s.transmit(&[3;1024]).unwrap();
        s.plain_udp=true;s.udp_gap=Duration::from_millis(20);
        s.next_udp_send=Instant::now()+Duration::from_secs(1);
        assert!(!s.flush().unwrap());assert_eq!(s.queued_bytes,1040);
        assert_eq!(s.outgoing_offset,0);assert_eq!(s.outgoing.len(),1);
    }

}
