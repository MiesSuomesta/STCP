"""Ensure loss/duplication cannot PASS the pattern, and new runs recover."""
import json,os,socket,struct,subprocess,sys,tempfile,time,threading
server,client=sys.argv[1:3]
def packet(run,seq,data,length=None):return struct.pack('>4I',0x42554432,run,seq,len(data) if length is None else length)+data
def pat(n):return bytes((i*31+0x17)&255 for i in range(n))
with tempfile.TemporaryDirectory() as root:
 with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as pick:pick.bind(('127.0.0.1',0));port=pick.getsockname()[1]
 path=os.path.join(root,'results.jsonl')
 proc=subprocess.Popen([server,'--transport','udp','--bind','127.0.0.1','--port',str(port),'--timeout-ms','200'],env=dict(os.environ,BEN2_RESULTS=path),stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
 try:
  time.sleep(.05)
  for run,fault in enumerate(['gap','duplicate','bad-length','oversize','timeout'],1):
   with socket.socket(socket.AF_INET,socket.SOCK_DGRAM) as s:
    s.connect(('127.0.0.1',port))
    s.send(packet(run,0,struct.pack('>5I',0x42454e32,2,1,1024,2048)))
    if fault=='gap':s.send(packet(run,2,pat(1024)))
    elif fault=='duplicate':s.send(packet(run,1,pat(1024)));s.send(packet(run,1,pat(1024)))
    elif fault=='bad-length':s.send(packet(run,1,pat(1024),900))
    elif fault=='oversize':s.send(packet(run,1,pat(1025)))
    time.sleep(.3)
   rows=[json.loads(line) for line in open(path)]
   assert rows[-1]['status']=='FAIL',(fault,rows[-1])
   assert len(rows)==run
   print('PASS server rejects',fault)
  good=subprocess.run([client,str(port),'65537','udp'],capture_output=True,text=True,timeout=10)
  assert good.returncode==0,good.stderr
  rows=[json.loads(line) for line in open(path)]
  assert len(rows)==8 and all(r['status']=='PASS' for r in rows[-3:])
  print('PASS server recovery after failed runs')
 finally:proc.terminate();proc.wait(timeout=3)
# Corrupt server direction: app must reject a dropped/duplicate/oversize datagram.
for fault in ['gap','duplicate','oversize']:
 s=socket.socket(socket.AF_INET,socket.SOCK_DGRAM);s.bind(('127.0.0.1',0));s.settimeout(5);port=s.getsockname()[1];errors=[]
 def serve():
  try:
   for mode in [1,2]:
    q,peer=s.recvfrom(1041);magic,run,seq,length=struct.unpack('>4I',q[:16]);assert seq==0
    if mode==1:
     total=struct.unpack('>5I',q[16:])[4];received=0
     while received<total:
      p,_=s.recvfrom(1041);received+=len(p)-16
     m,_=s.recvfrom(1041);assert len(m)==128
     s.sendto(packet(run,0,struct.pack('>5I',0x42454e32,1,0,total,0)),peer)
    else:
     data=bytes((i*31+0x53)&255 for i in range(1024))
     if fault=='gap':s.sendto(packet(run,1,data),peer)
     elif fault=='duplicate':s.sendto(packet(run,0,data),peer);s.sendto(packet(run,0,data),peer)
     else:s.sendto(packet(run,0,data+b'x'),peer)
  except Exception as e:errors.append(e)
 thread=threading.Thread(target=serve);thread.start()
 p=subprocess.run([client,str(port),'4096','udp'],capture_output=True,text=True,timeout=10)
 thread.join(6);s.close()
 assert not errors,errors
 assert p.returncode!=0,(fault,p.stderr)
 print('PASS app rejects',fault)
