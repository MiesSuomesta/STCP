import json,os,socket,subprocess,sys,tempfile,time
client,server=sys.argv[1:3]
carrier=int(sys.argv[3])
def port(kind):
 with socket.socket(socket.AF_INET,kind) as s:s.bind(('127.0.0.1',0));return s.getsockname()[1]
with tempfile.TemporaryDirectory() as root:
 for transport in ['tcp','udp','stcp-tcp','stcp-udp']:
  native='udp' if transport.endswith('udp') else 'tcp'
  p=port(socket.SOCK_DGRAM if native=='udp' else socket.SOCK_STREAM)
  path=os.path.join(root,transport+'.jsonl')
  with open(os.path.join(root,transport+'.log'),'w') as log:
   proc=subprocess.Popen([server,'--transport',transport if os.getenv('HOST_SERVER_SIMULATED') else native,'--bind','127.0.0.1','--port',str(p),'--timeout-ms','1500'],env=dict(os.environ,BEN2_RESULTS=path),stdout=log,stderr=log)
   try:
    time.sleep(.06)
    assert proc.poll() is None
    for total,chunk in ([(4096,4096),(4097,127)] if os.getenv('HOST_SHORT_SUITE') else [(4096,4096),(65537,4096),(1048576,4096),(4097,127)]):
     run=subprocess.run([client,str(p),str(total),transport,str(chunk)],capture_output=True,text=True,timeout=20)
     assert run.returncode==0,(transport,total,run.stderr)
     rows=[json.loads(line) for line in open(path)]
     assert len(rows)%3==0
     for mode,row in zip([1,2,3],rows[-3:]):
      assert row['status']=='PASS',row
      assert row['mode']==mode and row['total_bytes']==total
      assert row['client']['carrier']==('lte' if carrier==2 else 'ethernet')
      assert row['server']['rx_bytes']==(0 if mode==2 else total)
      assert row['server']['tx_bytes']==(0 if mode==1 else total)
      assert row['client']['tx_bytes']==row['server']['rx_bytes']
      assert row['client']['rx_bytes']==row['server']['tx_bytes']
      assert row['client']['errors']==0
     print('PASS',transport,total,chunk,'carrier',carrier,flush=True)
   finally:
    proc.terminate();proc.wait(timeout=3)
