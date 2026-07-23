#!/usr/bin/env python3
"""TCP/TLS/STCP throughput server with periodic progress reporting."""
from __future__ import annotations
import argparse, ipaddress, logging, signal, socket, ssl, struct, threading, time
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Sequence

AF_STCP=45; STCP_PROTOCOL=253; MAGIC=0x42454E32; VERSION=1
UPLOAD=1; DOWNLOAD=2; FULL=3
REQUEST=struct.Struct("!IIIII"); REPLY=struct.Struct("!IIIII")
MAX_TOTAL=64*1024*1024; MAX_CHUNK=64*1024
REPORT_EVERY=5.0


class IPWhitelist:
    def __init__(self, entries: Sequence[str]):
        self.networks = []
        for entry in entries:
            value = entry.strip()
            if not value or value.startswith("#"):
                continue
            try:
                self.networks.append(ipaddress.ip_network(value, strict=False))
            except ValueError as exc:
                raise ValueError(f"invalid whitelist entry {value!r}: {exc}") from exc

    @property
    def enabled(self) -> bool:
        return bool(self.networks)

    def allows(self, address) -> bool:
        if not self.enabled:
            return True
        host = address[0] if isinstance(address, tuple) else str(address)
        try:
            ip = ipaddress.ip_address(host)
        except ValueError:
            return False
        if isinstance(ip, ipaddress.IPv6Address) and ip.ipv4_mapped:
            ip = ip.ipv4_mapped
        return any(ip.version == net.version and ip in net for net in self.networks)

    def describe(self) -> str:
        return ", ".join(str(net) for net in self.networks) if self.networks else "disabled (allow all)"

def load_whitelist(entries, filename):
    values = list(entries or [])
    if filename:
        try:
            values.extend(Path(filename).read_text(encoding="utf-8").splitlines())
        except OSError as exc:
            raise ValueError(f"cannot read whitelist file {filename}: {exc}") from exc
    return IPWhitelist(values)

@dataclass(frozen=True)
class ListenerConfig:
    name:str; host:str; port:int; family:int; protocol:int=0; tls_context:Optional[ssl.SSLContext]=None

def tune_client_socket(c: socket.socket) -> None:
    """Best-effort throughput tuning for accepted TCP sockets."""
    for opt, value, name in (
        (socket.SO_RCVBUF, 1024 * 1024, "SO_RCVBUF"),
        (socket.SO_SNDBUF, 1024 * 1024, "SO_SNDBUF"),
    ):
        try:
            c.setsockopt(socket.SOL_SOCKET, opt, value)
            actual = c.getsockopt(socket.SOL_SOCKET, opt)
            logging.debug("client socket %s=%d", name, actual)
        except OSError as exc:
            logging.debug("client socket %s tuning unavailable: %s", name, exc)
    if hasattr(socket, "TCP_QUICKACK"):
        try:
            c.setsockopt(socket.IPPROTO_TCP, socket.TCP_QUICKACK, 1)
        except OSError as exc:
            logging.debug("TCP_QUICKACK unavailable: %s", exc)

class Progress:
    def __init__(self,label,total,interval):
        self.label=label; self.total=total; self.interval=interval
        self.started=time.monotonic(); self.last=self.started
    def update(self,done,force=False):
        now=time.monotonic()
        if not force and self.interval<=0: return
        if not force and now-self.last<self.interval: return
        elapsed=max(now-self.started,1e-6)
        remaining=max(self.total-done,0)
        pct=(done*100.0/self.total) if self.total else 100.0
        rate=done*8.0/elapsed/1000.0
        logging.info("%s progress=%5.1f%% transferred=%d/%d bytes remaining=%d rate=%.1f kbit/s",
                     self.label,pct,done,self.total,remaining,rate)
        self.last=now

def recv_exact(c:socket.socket,n:int)->bytes:
    out=bytearray()
    while len(out)<n:
        b=c.recv(n-len(out))
        if not b: raise ConnectionError("peer closed")
        out.extend(b)
    return bytes(out)

def pattern(offset:int,n:int,salt:int)->bytes:
    return bytes((((offset+i)*31+salt)&255) for i in range(n))

def recv_stream(c,total,chunk,salt,label):
    off=0; progress=Progress(label,total,REPORT_EVERY)
    buf=bytearray(min(max(chunk,4096),65536))
    view=memoryview(buf)
    while off<total:
        wanted=min(len(buf),total-off)
        n=c.recv_into(view,wanted)
        if n==0:
            raise ConnectionError(f"peer closed after {off}/{total} bytes")
        received=view[:n]
        expected=pattern(off,n,salt)
        if received.tobytes()!=expected:
            raise ValueError(f"pattern mismatch at {off}")
        off+=n; progress.update(off,off==total)

def send_stream(c,total,chunk,salt,label):
    off=0; progress=Progress(label,total,REPORT_EVERY)
    while off<total:
        n=min(chunk,total-off); c.sendall(pattern(off,n,salt)); off+=n
        progress.update(off,off==total)

def reply(c,mode,status=0,rx=0,tx=0): c.sendall(REPLY.pack(MAGIC,mode,status,rx,tx))

def handle_client(name,c,address):
    tune_client_socket(c)
    logging.info("%s client connected: %s",name,address)
    started=time.monotonic(); mode=0; total=0
    try:
        c.settimeout(180.0)
        magic,version,mode,chunk,total=REQUEST.unpack(recv_exact(c,REQUEST.size))
        if magic!=MAGIC or version!=VERSION: raise ValueError("bad request")
        if not 1<=chunk<=MAX_CHUNK or not 1<=total<=MAX_TOTAL: raise ValueError("bad sizes")
        names={UPLOAD:"UPLOAD",DOWNLOAD:"DOWNLOAD",FULL:"FULL"}
        logging.info("%s %s starting %s total=%d bytes chunk=%d bytes",name,address,names.get(mode,mode),total,chunk)
        started=time.monotonic()
        if mode==UPLOAD:
            recv_stream(c,total,chunk,0x17,f"{name} {address} UPLOAD RX"); reply(c,mode,rx=total)
        elif mode==DOWNLOAD:
            send_stream(c,total,chunk,0x53,f"{name} {address} DOWNLOAD TX")
            am,amode,status,rx,tx=REPLY.unpack(recv_exact(c,REPLY.size))
            if am!=MAGIC or amode!=mode or status or rx!=total: raise ValueError("bad download ack")
            reply(c,mode,tx=total)
        elif mode==FULL:
            err=[]
            def sender():
                try:
                    send_stream(c,total,chunk,0x53,f"{name} {address} FULL TX")
                except Exception as e:
                    err.append(e)
            t=threading.Thread(target=sender,name=f"{name}-full-tx")
            t.start()
            recv_stream(c,total,chunk,0x17,f"{name} {address} FULL RX")
            t.join()
            if err:
                raise err[0]
            am,amode,status,rx,tx=REPLY.unpack(recv_exact(c,REPLY.size))
            if am!=MAGIC or amode!=mode or status or rx!=total or tx!=total:
                raise ValueError("bad full-duplex completion ack")
            reply(c,mode,rx=total,tx=total)
        else: raise ValueError(f"bad mode {mode}")
        elapsed=max(time.monotonic()-started,1e-6)
        logging.info("%s %s %s complete bytes=%d elapsed=%.3fs TX/RX aggregate=%.1f/%.1f/%.1f kbit/s",
            name,address,names[mode],total,elapsed,
            (total*8/elapsed/1000 if mode in (DOWNLOAD,FULL) else 0),
            (total*8/elapsed/1000 if mode in (UPLOAD,FULL) else 0),
            (total*8/elapsed/1000*(2 if mode==FULL else 1)))
    except Exception:
        logging.exception("%s client error: %s",name,address)
        try:
            if mode: reply(c,mode,1)
        except Exception: pass
    finally:
        c.close(); logging.info("%s client disconnected: %s",name,address)

def listener_loop(cfg,stop,whitelist):
    try: s=socket.socket(cfg.family,socket.SOCK_STREAM,cfg.protocol)
    except OSError as e: logging.error("%s socket creation failed: %s",cfg.name,e); return
    with s:
        s.setsockopt(socket.SOL_SOCKET,socket.SO_REUSEADDR,1)
        try:
            s.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024 * 1024)
            s.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 1024 * 1024)
        except OSError:
            pass
        s.bind((cfg.host,cfg.port)); s.listen(64); s.settimeout(1)
        logging.info("%s listening on %s:%d",cfg.name,cfg.host,cfg.port)
        while not stop.is_set():
            try: c,a=s.accept()
            except socket.timeout: continue
            if not whitelist.allows(a):
                logging.warning("%s rejected non-whitelisted client: %s", cfg.name, a)
                try:
                    c.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                c.close()
                continue
            logging.info("%s accepted whitelisted client: %s", cfg.name, a)
            if cfg.tls_context:
                try: c=cfg.tls_context.wrap_socket(c,server_side=True)
                except ssl.SSLError as e: logging.warning("TLS handshake failed: %s",e); c.close(); continue
            threading.Thread(target=handle_client,args=(cfg.name,c,a),daemon=True).start()

def tls_context(cert,key):
    x=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER); x.minimum_version=ssl.TLSVersion.TLSv1_2; x.load_cert_chain(cert,key); return x

def main():
    global REPORT_EVERY
    p=argparse.ArgumentParser(); p.add_argument("--host",default="0.0.0.0"); p.add_argument("--allow",action="append",default=[],metavar="IP_OR_CIDR",help="allow one client IP or CIDR; may be repeated"); p.add_argument("--allow-file",type=Path,metavar="PATH",help="read allowed IPs/CIDRs from file, one per line; # comments allowed"); p.add_argument("--tcp-port",type=int,default=19000); p.add_argument("--tls-port",type=int,default=19001); p.add_argument("--stcp-port",type=int,default=19002); p.add_argument("--cert",type=Path,default=Path("cert.pem")); p.add_argument("--key",type=Path,default=Path("key.pem")); p.add_argument("--no-tls",action="store_true"); p.add_argument("--no-stcp",action="store_true"); p.add_argument("--verbose",action="store_true"); p.add_argument("--report-every",type=float,default=5.0,metavar="SECONDS",help="progress reporting interval; 0 disables periodic reports"); a=p.parse_args()
    if a.report_every<0: p.error("--report-every must be >= 0")
    REPORT_EVERY=a.report_every
    try:
        whitelist=load_whitelist(a.allow,a.allow_file)
    except ValueError as exc:
        p.error(str(exc))
    logging.basicConfig(level=logging.DEBUG if a.verbose else logging.INFO,format="%(asctime)s %(levelname)s %(message)s")
    logging.info("Progress reporting interval: %.1f seconds",REPORT_EVERY)
    logging.info("IP whitelist: %s", whitelist.describe())
    stop=threading.Event(); signal.signal(signal.SIGINT,lambda *_:stop.set()); signal.signal(signal.SIGTERM,lambda *_:stop.set())
    cfg=[ListenerConfig("TCP",a.host,a.tcp_port,socket.AF_INET)]
    if not a.no_tls:
        if not a.cert.exists() or not a.key.exists(): logging.error("TLS cert missing"); return 2
        cfg.append(ListenerConfig("TLS",a.host,a.tls_port,socket.AF_INET,tls_context=tls_context(a.cert,a.key)))
    if not a.no_stcp: cfg.append(ListenerConfig("STCP",a.host,a.stcp_port,AF_STCP,STCP_PROTOCOL))
    ts=[threading.Thread(target=listener_loop,args=(x,stop,whitelist),daemon=True) for x in cfg]
    [t.start() for t in ts]
    while not stop.wait(1):
        if not any(t.is_alive() for t in ts): return 1
    return 0
if __name__=="__main__": raise SystemExit(main())
