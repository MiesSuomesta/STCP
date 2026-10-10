#!/usr/bin/env python3
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parent
p=Path(sys.argv[1]) if len(sys.argv)>1 else root/'application/build-ethernet/application/zephyr/.config'
if not p.is_file():sys.exit('Missing generated config: '+str(p))
s=p.read_text();m=re.search(r'^CONFIG_STCP_BUFFER_POOL_FRAMES=(\d+)$',s,re.M)
if not m or m[1]!='16':sys.exit('FAIL: built pool is not16; do not flash. Check selected config/build directory.')
print('PASS generated pool=16:',p)
for symbol in ['STCP_BUFFER_POOL_LARGE','STCP_RUST_RX_BUFFER_SIZE']:
 m=re.search(r'^CONFIG_'+symbol+r'=(\d+)$',s,re.M)
 if m:print(symbol+'='+m[1])
if 'CONFIG_STCP_BUFFER_POOL_LARGE=2' in s:print('Static buffer storage:34816 bytes (34KiB).')
