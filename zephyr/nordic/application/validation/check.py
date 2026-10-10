import socket,struct,threading,subprocess,sys
binary=sys.argv[1]
carrier=int(sys.argv[2])
def exact(s,n):
 data=bytearray()
 while len(data)<n:
  part=s.recv(n-len(data))
  if not part:raise AssertionError('early EOF')
  data.extend(part)
 return bytes(data)
def pat(n,salt):return bytes((i*31+salt)&255 for i in range(n))
for total in [4096,65537,1048576]:
 listener=socket.socket();listener.bind(('127.0.0.1',0));listener.listen();listener.settimeout(10)
 port=listener.getsockname()[1];errors=[];reports=[]
 def serve():
  try:
   for mode in [1,2,3]:
    s,_=listener.accept();s.settimeout(10)
    with s:
     assert struct.unpack('>5I',exact(s,20))==(0x42454e32,2,mode,4096,total)
     def sending():
      try:
       data=pat(total,0x53)
       for off in range(0,total,713):s.sendall(data[off:off+713])
      except Exception as e:errors.append(e)
     sender=None
     if mode!=1:
      sender=threading.Thread(target=sending);sender.start()
     if mode!=2:assert exact(s,total)==pat(total,0x17)
     if sender:sender.join()
     if mode!=1:assert struct.unpack('>5I',exact(s,20))==(0x42454e32,mode,0,total,total if mode==3 else 0)
     report=struct.unpack('>4I12Q',exact(s,112));reports.append(report)
     assert report[:4]==(0x4d545232,1,112,carrier)
     assert report[4]==(0 if mode==2 else total)
     assert report[5]==(0 if mode==1 else total)
     assert report[11]==0 # errors
     if mode!=2:assert report[6]>0 and report[8]>0
     if mode!=1:assert report[7]>0 and report[9]>0
     s.sendall(struct.pack('>5I',0x42454e32,mode,0,total if mode!=2 else 0,total if mode!=1 else 0))
  except Exception as e:errors.append(e)
 thread=threading.Thread(target=serve);thread.start()
 proc=subprocess.run([binary,str(port),str(total)],capture_output=True,text=True,timeout=30)
 thread.join(timeout=12);listener.close()
 assert not thread.is_alive()
 assert not errors,errors
 assert proc.returncode==0,(proc.returncode,proc.stderr)
 assert len(reports)==3
 print('PASS upload/download/full:',total,'bytes carrier=',carrier)
