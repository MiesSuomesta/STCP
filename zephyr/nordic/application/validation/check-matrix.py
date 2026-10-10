import socket,struct,threading,subprocess,sys
binary=sys.argv[1]
class Connection:
 def __init__(self,s,udp):self.s=s;self.udp=udp;self.run=None;self.peer=None;self.rx=0;self.tx=0;self.pending=bytearray()
 def read(self,n):
  while len(self.pending)<n:
   if self.udp:
    p,peer=self.s.recvfrom(1041);magic,run,seq,length=struct.unpack('>4I',p[:16]);assert magic==0x42554432 and seq==self.rx and len(p)==length+16
    if self.run is None:self.run=run;self.peer=peer
    assert self.run==run and self.peer==peer;self.rx+=1;self.pending.extend(p[16:])
   else:
    p=self.s.recv(n-len(self.pending));assert p;self.pending.extend(p)
  out=bytes(self.pending[:n]);del self.pending[:n];return out
 def write(self,data):
  if self.udp:
   p=struct.pack('>4I',0x42554432,self.run,self.tx,len(data))+data;self.s.sendto(p,self.peer);self.tx+=1
  else:self.s.sendall(data)
def pat(n,salt):return bytes((i*31+salt)&255 for i in range(n))
for failure in [False,True]:
 listeners=[]
 for base in range(25000,26000,4):
  try:
   for t in range(4):
    s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM if t in [1,3] else socket.SOCK_STREAM);s.bind(('127.0.0.1',base+t));s.settimeout(5)
    if t in [0,2]:s.listen()
    listeners.append(s)
   break
  except OSError:
   for s in listeners:s.close()
   listeners=[]
 assert len(listeners)==4
 errors=[];record=[]
 def server(t):
  try:
   listener=listeners[t]
   for mode in [1,2,3]:
    s=listener if t in [1,3] else listener.accept()[0];s.settimeout(5);c=Connection(s,t in [1,3])
    assert struct.unpack('>5I',c.read(20))==(0x42454e32,2,mode,4096,4096)
    if failure and t==0 and mode==1:s.close();continue
    if mode!=1:
     for off in range(0,4096,1024):c.write(pat(4096,0x53)[off:off+1024])
    if mode!=2:assert c.read(4096)==pat(4096,0x17)
    if mode!=1:assert struct.unpack('>5I',c.read(20))==(0x42454e32,mode,0,4096,4096 if mode==3 else 0)
    m=struct.unpack('>4I12Q',c.read(112));assert m[:3]==(0x4d545232,1,112)
    c.write(struct.pack('>5I',0x42454e32,mode,0,4096 if mode!=2 else 0,4096 if mode!=1 else 0))
    if t in [0,2]:s.close()
   if t==2:
    s,_=listener.accept();s.settimeout(5);c=Connection(s,False);data=c.read(224);v=struct.unpack('>56I',data);record.append(v)
    assert v[:4]==(0x464f4d32,1,224,4)
    for transport in range(4):
     assert v[4+13*transport]==transport
     for mode in range(3):
      at=5+13*transport+4*mode
      if failure and transport==0 and mode==0:assert v[at]!=0
      else:
       assert v[at]==0
       assert v[at+1]==(0 if mode==1 else 4096)
       assert v[at+2]==(0 if mode==0 else 4096)
       assert v[at+3]>0
    c.write(struct.pack('>5I',0x464f4d32,1,0,0,0));s.close()
  except Exception as e:errors.append((t,e))
 threads=[threading.Thread(target=server,args=(t,)) for t in range(4)]
 for t in threads:t.start()
 p=subprocess.run([binary,str(base)],capture_output=True,text=True,timeout=20)
 for t in threads:t.join(6)
 for s in listeners:s.close()
 assert not errors,errors
 assert all(not t.is_alive() for t in threads)
 assert p.returncode==(1 if failure else 0),(p.returncode,p.stderr)
 assert len(record)==1
 print('PASS stcp bench matrix, failure injected=',failure)
