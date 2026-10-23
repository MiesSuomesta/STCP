#!/usr/bin/env python3
"""Run production P2P/Noise core with host OpenSSL crypto, outside the kernel."""
import argparse,json,os,platform,shutil,subprocess,tempfile
from pathlib import Path

def main():
 ap=argparse.ArgumentParser(description=__doc__)
 ap.add_argument('--cargo',default='cargo');ap.add_argument('--cc',default='cc')
 a=ap.parse_args();here=Path(__file__).resolve().parent;module=here.parents[1]
 target={'x86_64':'x86_64-unknown-linux-gnu','aarch64':'aarch64-unknown-linux-gnu'}.get(platform.machine())
 if target is None:raise SystemExit('Unsupported host architecture')
 with tempfile.TemporaryDirectory(prefix='stcp-p2p-host-') as tmp:
  t=Path(tmp);obj=t/'host_crypto.o'
  subprocess.run([a.cc,'-std=c11','-O2','-Wall','-Wextra','-Werror','-c',str(here/'host_crypto.c'),'-o',str(obj)],check=True)
  (t/'Cargo.toml').write_text('[package]\nname="stcp-kernel-core"\nversion="0.1.0"\nedition="2024"\n[workspace]\n[lib]\npath="harness.rs"\n[dependencies]\ned25519-compact={version="2.3.1",default-features=false}\n')
  shutil.copy2(module/'rust/Cargo.lock',t/'Cargo.lock')
  root='extern crate alloc;\n'+''.join(f'#[path={json.dumps(str(module/"rust/src"/(m+".rs")))}] mod {m};\n' for m in ['error','kdf','p2p'])
  (t/'harness.rs').write_text(root+(here/'tests.rs').read_text())
  env=os.environ.copy();env['RUSTFLAGS']=f'-A dead_code -C link-arg={obj} -C link-arg=-lcrypto'
  subprocess.run([a.cargo,'test','--locked','--manifest-path',str(t/'Cargo.toml'),'--target',target,'--lib'],check=True,env=env,cwd=t)
if __name__=='__main__':main()
