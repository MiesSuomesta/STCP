#!/usr/bin/env python3
"""Run from package placed in zephyr/nordic; preserve all other settings."""
from pathlib import Path
import re
import sys
root=Path(__file__).resolve().parent
module=root/'module-v2'
config=module/'Kconfig'
eth=root/'ethernet.conf'
if not config.is_file() or not eth.is_file():
    sys.exit('Extract this package into zephyr/nordic (beside module-v2 and ethernet.conf).')
s=config.read_text()
pattern=r'(config STCP_BUFFER_POOL_FRAMES\n)(.*?)(?=\n(?:config|menuconfig|endmenu|endif)\b|\Z)'
m=re.search(pattern,s,re.S)
if not m:sys.exit('STCP_BUFFER_POOL_FRAMES not found; install frame pool module first.')
body,n=re.subn(r'(?m)^(\s*default)\s+\d+\s*$',r'\g<1> 16',m[2])
if n!=1:sys.exit('Expected one numeric pool default; no changes written.')
new=s[:m.start(2)]+body+s[m.end(2):]
lines=eth.read_text().splitlines()
lines=[line for line in lines if not re.match(r'^\s*(?:CONFIG_STCP_BUFFER_POOL_FRAMES\s*=|#\s*CONFIG_STCP_BUFFER_POOL_FRAMES\s+is not set)',line)]
lines.append('CONFIG_STCP_BUFFER_POOL_FRAMES=16')
# All validation before writing. Only pool settings are changed.
config.write_text(new)
eth.write_text('\n'.join(lines)+'\n')
print('Applied frame pool=16 to',config,'and',eth)
print('Rust session.rs untouched: keep SEND_WINDOW=8, ACK_EVERY_FRAMES=4 on both endpoints.')
