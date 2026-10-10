//! Linux BEN2 native socket server. Standard library only; STCP uses stcp.ko.
use std::{io, mem, os::fd::{AsRawFd, FromRawFd, OwnedFd}, time::{Duration,Instant}};

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
    fn connect(fd:i32,address:*const Address,len:u32)->i32;
    fn shutdown(fd:i32,how:i32)->i32;
    fn socket(domain:i32,kind:i32,protocol:i32)->i32;
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
    tx_calls:u64,rx_calls:u64,rx_eagain:u64,
}
impl Socket {
    fn new(fd:OwnedFd,transport:&str,timeout_ms:u64)->Result<Self,String> {
        let raw=fd.as_raw_fd();
        let tv=Timeval{sec:(timeout_ms/1000)as _,usec:((timeout_ms%1000)*1000)as _};
        checked(unsafe{setsockopt(raw,1,21,(&tv as *const Timeval).cast(),mem::size_of::<Timeval>()as u32)})?;
        Ok(Self{fd,plain_udp:transport=="udp",framed:transport.ends_with("udp"),peer:None,run:None,tx_seq:0,rx_seq:0,pending:Vec::with_capacity(2081),tx_calls:0,rx_calls:0,rx_eagain:0})
    }
    fn receive(&mut self,buffer:&mut [u8])->Result<Option<usize>,String> {
        if self.framed && !self.plain_udp {
            if let Some(n)=self.extract(buffer)? {return Ok(Some(n));}
        }
        let mut raw=[0u8;16384];let capacity=if self.framed {1041}else{raw.len()};
        let mut peer=Address::default();let mut size=mem::size_of::<Address>()as u32;
        let n=unsafe{if self.plain_udp {recvfrom(self.fd.as_raw_fd(),raw.as_mut_ptr(),capacity,MSG_DONTWAIT,&mut peer,&mut size)}
            else {recv(self.fd.as_raw_fd(),raw.as_mut_ptr(),capacity,MSG_DONTWAIT)}};
        self.rx_calls+=1;
        if n<0 {
            let e=io::Error::last_os_error();
            if e.kind()==io::ErrorKind::WouldBlock {self.rx_eagain+=1;}
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
        let mut offset=0;
        while offset<bytes.len() {
            let n=unsafe{if self.plain_udp {
                let peer=self.peer.ok_or("UDP send before peer")?;
                sendto(self.fd.as_raw_fd(),bytes.as_ptr(),bytes.len(),MSG_NOSIGNAL,&peer,mem::size_of::<Address>()as u32)
            }else{send(self.fd.as_raw_fd(),bytes[offset..].as_ptr(),bytes.len()-offset,MSG_NOSIGNAL)}};
            self.tx_calls+=1;
            if n<0 {let e=io::Error::last_os_error();if e.kind()==io::ErrorKind::Interrupted {continue;}return Err(e.to_string());}
            if n==0 {return Err("zero-length socket send".into());}
            if self.plain_udp && n as usize!=bytes.len() {return Err("partial UDP send".into());}
            offset+=n as usize;
        }
        if self.framed {self.tx_seq=self.tx_seq.checked_add(1).ok_or("UDP sequence overflow")?;}
        Ok(())
    }
    fn wait(&self,ms:i32)->Result<(),String> {
        let mut p=PollFd{fd:self.fd.as_raw_fd(),events:1,revents:0};
        let rc=unsafe{poll(&mut p,1,ms)};
        if rc<0 && io::Error::last_os_error().kind()!=io::ErrorKind::Interrupted {return Err(error());}Ok(())
    }
}

const MAGIC:u32=0x42454e32;
#[derive(Clone,Copy,Default)]
struct Counters {bytes:u64,us:u64,calls:u64,eagain:u64}
struct Reader {buffer:Vec<u8>,offset:usize}
impl Reader {
    fn new()->Self {Self{buffer:Vec::new(),offset:0}}
    fn exact(&mut self,socket:&mut Socket,n:usize,timeout:Duration)->Result<Vec<u8>,String> {
        let mut out=Vec::with_capacity(n);let mut activity=Instant::now();
        while out.len()<n {
            if self.offset<self.buffer.len() {
                let count=(n-out.len()).min(self.buffer.len()-self.offset);
                out.extend_from_slice(&self.buffer[self.offset..self.offset+count]);self.offset+=count;
                if self.offset==self.buffer.len(){self.buffer.clear();self.offset=0;}
                continue;
            }
            let mut data=[0u8;16384];
            if let Some(got)=socket.receive(&mut data)? {self.buffer.extend_from_slice(&data[..got]);activity=Instant::now();}
            else {if activity.elapsed()>=timeout{return Err("receive inactivity timeout".into());}socket.wait(20)?;}
        }
        Ok(out)
    }
}
fn micros(t:Instant)->u64 {t.elapsed().as_micros().min(u64::MAX as u128)as u64}
fn words(values:&[u32])->Vec<u8> {values.iter().flat_map(|v|v.to_be_bytes()).collect()}
fn pattern(offset:u32,salt:u8)->u8 {(offset.wrapping_mul(31).wrapping_add(salt as u32)&255)as u8}
fn upload(socket:&mut Socket,total:u32,chunk:u32)->Result<Counters,String> {
    let start=Instant::now();let begin_calls=socket.tx_calls;let mut stats=Counters::default();let mut offset=0;
    let count=chunk.min(1024);let mut data=[0u8;1024];
    while offset<total {
        let n=count.min(total-offset)as usize;
        for(i,b)in data[..n].iter_mut().enumerate(){*b=pattern(offset+i as u32,0x17);}
        socket.transmit(&data[..n])?;stats.bytes+=n as u64;offset+=n as u32;
    }
    stats.us=micros(start);stats.calls=socket.tx_calls-begin_calls;Ok(stats)
}
fn download(reader:&mut Reader,socket:&mut Socket,total:u32,chunk:u32,timeout:Duration)->Result<Counters,String> {
    let start=Instant::now();let begin_calls=socket.rx_calls;let begin_eagain=socket.rx_eagain;let mut stats=Counters::default();let mut offset=0;
    while offset<total {
        let n=chunk.min(1024).min(total-offset)as usize;
        let data=reader.exact(socket,n,timeout)?;
        for(i,&b)in data.iter().enumerate(){if b!=pattern(offset+i as u32,0x53){return Err(format!("payload mismatch at {}",offset+i as u32));}}
        stats.bytes+=n as u64;offset+=n as u32;
    }
    stats.us=micros(start);stats.calls=socket.rx_calls-begin_calls;stats.eagain=socket.rx_eagain-begin_eagain;Ok(stats)
}
fn metric(tx:Counters,rx:Counters,connect_us:u64,elapsed_us:u64)->Vec<u8> {
    let mut out=words(&[0x4d545232,1,112,0]); // unknown carrier: Linux host test, not Zephyr LTE metadata
    for v in [tx.bytes,rx.bytes,tx.us,rx.us,tx.calls,rx.calls,tx.eagain+rx.eagain,0,0,0,connect_us,elapsed_us] {out.extend_from_slice(&v.to_be_bytes());}
    out
}
fn clone_sender(socket:&Socket,transport:&str,timeout_ms:u64)->Result<Socket,String> {
    let fd=socket.fd.try_clone().map_err(|e|e.to_string())?;
    let mut sender=Socket::new(fd,transport,timeout_ms)?;
    sender.peer=socket.peer;sender.run=socket.run;sender.tx_seq=socket.tx_seq;Ok(sender)
}
/// FULL is aggregate upload+download payload rate, excluding connect/control traffic.
pub fn test(host:std::net::Ipv4Addr,port:u16,transport:&str,mode:u32,total:u32,chunk:u32,timeout_ms:u64)->Result<f64,String> {
    let (family,kind,proto)=match transport{"tcp"=>(AF_INET,1,6),"udp"=>(AF_INET,2,17),"stcp-tcp"=>(AF_STCP,1,253),"stcp-udp"=>(AF_STCP,1,254),_=>return Err("invalid transport".into())};
    let fd=owned(unsafe{socket(family,kind,proto)})?;
    let mut socket=Socket::new(fd,transport,timeout_ms)?;
    let queue=4*1024*1024i32;unsafe{setsockopt(socket.fd.as_raw_fd(),1,8,(&queue as *const i32).cast(),4)};
    let address=Address{family:AF_INET as u16,port:port.to_be(),addr:u32::from_ne_bytes(host.octets()),zero:[0;8]};
    let started=Instant::now();checked(unsafe{connect(socket.fd.as_raw_fd(),&address,mem::size_of::<Address>()as u32)})?;
    let connect_us=micros(started);
    if socket.framed {
        socket.run=Some(std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).map_err(|e|e.to_string())?.as_nanos()as u32);
        if socket.plain_udp {socket.peer=Some(address);}
    }
    socket.transmit(&words(&[MAGIC,2,mode,chunk,total]))?;
    let timeout=Duration::from_millis(timeout_ms);let mut reader=Reader::new();
    let start=Instant::now();let(mut tx,mut rx)=(Counters::default(),Counters::default());
    match mode {
        1=>tx=upload(&mut socket,total,chunk)?,
        2=>rx=download(&mut reader,&mut socket,total,chunk,timeout)?,
        3=>{
            let mut sender=clone_sender(&socket,transport,timeout_ms)?;
            let thread=std::thread::spawn(move||{
                let result=upload(&mut sender,total,chunk);(result,sender.tx_seq)
            });
            let receiving=download(&mut reader,&mut socket,total,chunk,timeout);
            if receiving.is_err() {unsafe{shutdown(socket.fd.as_raw_fd(),2)};}
            let (sending,seq)=thread.join().map_err(|_|"upload worker panicked")?;
            socket.tx_seq=seq;rx=receiving?;tx=sending?;
        },_=>return Err("invalid mode".into()),
    }
    let elapsed_us=micros(start);
    if mode!=1 {socket.transmit(&words(&[MAGIC,mode,0,rx.bytes as u32,tx.bytes as u32]))?;}
    socket.transmit(&metric(tx,rx,connect_us,elapsed_us))?;
    let reply=reader.exact(&mut socket,20,timeout)?;
    if [word(&reply,0),word(&reply,4),word(&reply,8),word(&reply,12),word(&reply,16)] != [MAGIC,mode,0,tx.bytes as u32,rx.bytes as u32] {return Err(format!("invalid server reply {:02x?}",reply));}
    let us=match mode {1=>tx.us,2=>rx.us,_=>elapsed_us};
    if us==0 {return Err("zero measurement duration".into());}
    Ok((tx.bytes+rx.bytes)as f64*8.0*1e6/us as f64)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::{net::TcpListener,io::{Read,Write},os::unix::net::UnixStream};
    fn envelope(run:u32,seq:u32,data:&[u8])->Vec<u8> {
        let mut out=words(&[BUD2,run,seq,data.len()as u32]);out.extend_from_slice(data);out
    }
    #[test]
    fn framing_preserves_split_and_coalesced_packets() {
        let(a,_b)=UnixStream::pair().unwrap();let mut socket=Socket::new(a.into(),"stcp-udp",1000).unwrap();socket.run=Some(7);
        let one=envelope(7,0,&[3;100]);let two=envelope(7,1,&[4;20]);let mut out=[0;1024];
        socket.pending.extend_from_slice(&one[..8]);assert_eq!(socket.extract(&mut out).unwrap(),None);
        socket.pending.extend_from_slice(&one[8..]);socket.pending.extend_from_slice(&two);
        assert_eq!(socket.extract(&mut out).unwrap(),Some(100));assert_eq!(out[..100],[3;100]);
        assert_eq!(socket.extract(&mut out).unwrap(),Some(20));assert_eq!(out[..20],[4;20]);
        socket.pending.extend(envelope(7,1,&[0;20]));assert!(socket.extract(&mut out).is_err());
    }
    #[test]
    fn metrics_are_112_byte_big_endian_raw_record() {
        let m=metric(Counters{bytes:1024,us:200,calls:2,eagain:0},Counters::default(),7,300);
        assert_eq!(m.len(),112);assert_eq!(word(&m,0),0x4d545232);assert_eq!(word(&m,8),112);
        assert_eq!(u64::from_be_bytes(m[16..24].try_into().unwrap()),1024);
        assert_eq!(u64::from_be_bytes(m[96..104].try_into().unwrap()),7);
    }
    #[test]
    fn native_tcp_all_directions_against_reference() {
        let listener=TcpListener::bind("127.0.0.1:0").unwrap();let port=listener.local_addr().unwrap().port();
        let worker=std::thread::spawn(move||{
            for mode in 1..=3u32 {
                let(mut stream,_)=listener.accept().unwrap();stream.set_read_timeout(Some(Duration::from_secs(3))).unwrap();
                let mut req=[0u8;20];stream.read_exact(&mut req).unwrap();
                assert_eq!(word(&req,0),MAGIC);assert_eq!(word(&req,4),2);assert_eq!(word(&req,8),mode);
                let total=word(&req,16)as usize;
                let sender=if mode!=1 {let mut s=stream.try_clone().unwrap();Some(std::thread::spawn(move||{
                    let bytes:Vec<u8>=(0..total).map(|i|pattern(i as u32,0x53)).collect();
                    for part in bytes.chunks(113){s.write_all(part).unwrap();}
                }))}else{None};
                if mode!=2 {let mut data=vec![0;total];stream.read_exact(&mut data).unwrap();
                    assert!(data.iter().enumerate().all(|(i,&b)|b==pattern(i as u32,0x17)));}
                if let Some(t)=sender {t.join().unwrap();}
                if mode!=1 {let mut ack=[0;20];stream.read_exact(&mut ack).unwrap();assert_eq!(word(&ack,0),MAGIC);assert_eq!(word(&ack,8),0);}
                let mut m=[0;112];stream.read_exact(&mut m).unwrap();assert_eq!(word(&m,0),0x4d545232);
                stream.write_all(&words(&[MAGIC,mode,0,if mode==2{0}else{total as u32},if mode==1{0}else{total as u32}])).unwrap();
            }
        });
        for mode in 1..=3 {assert!(test(std::net::Ipv4Addr::LOCALHOST,port,"tcp",mode,65537,4096,3000).unwrap()>0.0);}
        worker.join().unwrap();
    }
}
