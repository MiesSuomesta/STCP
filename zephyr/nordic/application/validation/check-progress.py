"""Rust server integration: slow upload, reporting interval, disabled reporting."""
import json,os,socket,struct,subprocess,sys,tempfile,time
server=sys.argv[1]
def exact(s,n):
 data=bytearray()
 while len(data)<n:
  p=s.recv(n-len(data))
  assert p,'early EOF'
  data.extend(p)
 return bytes(data)
with tempfile.TemporaryDirectory() as root:
 for interval in [100,0]:
  with socket.socket() as pick:pick.bind(('127.0.0.1',0));port=pick.getsockname()[1]
  path=os.path.join(root,str(interval)+'.log')
  with open(path,'w') as log:
   proc=subprocess.Popen([server,'--transport','tcp','--bind','127.0.0.1','--port',str(port),'--report-ms',str(interval),'--timeout-ms','2000'],stdout=log,stderr=log,env=dict(os.environ,BEN2_RESULTS=os.path.join(root,'results.jsonl')))
   try:
    deadline=time.monotonic()+5
    while True:
     try:s=socket.create_connection(('127.0.0.1',port),.2);break
     except OSError:
      assert proc.poll() is None
      if time.monotonic()>deadline:raise
      time.sleep(.03)
    with s:
     s.settimeout(3);total=8192;s.sendall(struct.pack('>5I',0x42454e32,2,1,1024,total))
     begin=time.monotonic()
     for off in range(0,total,1024):
      s.sendall(bytes(((off+i)*31+0x17)&255 for i in range(1024)));time.sleep(.09)
     micros=int((time.monotonic()-begin)*1e6)
     s.sendall(struct.pack('>4I12Q',0x4d545232,1,112,2,total,0,micros,0,8,0,0,0,0,0,0,micros))
     assert struct.unpack('>5I',exact(s,20))==(0x42454e32,1,0,total,0)
    time.sleep(.05)
   finally:proc.terminate();proc.wait(timeout=3)
  lines=open(path).read().splitlines()
  reports=[json.loads(x.removeprefix('BEN2_PROGRESS ')) for x in lines if x.startswith('BEN2_PROGRESS ')]
  results=[json.loads(x.removeprefix('BEN2_RESULT ')) for x in lines if x.startswith('BEN2_RESULT ')]
  assert len(results)==1 and results[0]['status']=='PASS',lines
  if interval:
   assert len(reports)>=4,lines
   assert all(r['sequence']==results[0]['sequence'] for r in reports)
   assert all(0<=r['rx_bytes']<=total and r['tx_bytes']==0 for r in reports)
   assert all(b['elapsed_us']-a['elapsed_us']>=interval*1000 for a,b in zip(reports,reports[1:]))
  else:assert not reports,lines
  print('PASS report-ms',interval,'progress rows',len(reports))
