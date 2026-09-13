#!/usr/bin/env python3
import argparse, json, os, random, re, shlex, signal, subprocess, sys, time, fcntl
from dataclasses import dataclass, asdict
from datetime import datetime, timezone
from pathlib import Path

try:
    import serial
except ImportError:
    print('ERROR: pyserial missing. Install: python3 -m pip install pyserial', file=sys.stderr)
    sys.exit(2)

JSON_BEGIN = re.compile(r'STCP_BENCH_JSON_BEGIN\s+(\d+)(?:\s+(\d+))?')
JSON_PART = re.compile(r'STCP_BENCH_JSON_PART\s+(?:(\d+)/(\d+)\s+)?(.*)')
JSON_END = re.compile(r'STCP_BENCH_JSON_END')
PROMPT = re.compile(r'(?:^|[\r\n])stcp>\s*', re.M)
SUCCESS = re.compile(r'(completed successfully|status\s*=\s*PASS)', re.I)
FAILURE = re.compile(r'(failed:\s*-?\d+|Benchmark failed:|status\s*=\s*FAILED)', re.I)
CRASH = re.compile(r'(ASSERTION FAIL|KERNEL PANIC|FATAL ERROR|HardFault|UsageFault|BusFault|MemManage Fault|Oops:|BUG:|KASAN|watchdog)', re.I)

@dataclass
class Result:
    seq: int
    started_at: str
    phase: str
    direction: str
    port: int
    chunk: int
    total: int
    idle_before_ms: int
    rc: str
    elapsed_s: float
    json_result: dict | None
    crash_signature: str | None = None

class ZephyrShell:
    def __init__(self, dev, baud, raw_log: Path, timeout=0.2):
        self.ser = serial.Serial(dev, baudrate=baud, timeout=timeout, write_timeout=2, exclusive=True)
        self.raw = raw_log.open('a', buffering=1, encoding='utf-8', errors='replace')
        self.buf = ''

    def close(self):
        try: self.ser.close()
        finally: self.raw.close()

    def _read(self, deadline):
        while time.monotonic() < deadline:
            b = self.ser.read(4096)
            if b:
                s = b.decode('utf-8', errors='replace')
                self.raw.write(s)
                self.raw.flush()
                self.buf += s
                yield s
            else:
                time.sleep(0.02)

    def send(self, cmd):
        self.raw.write(f'\n\n### HOST_SEND {datetime.now(timezone.utc).isoformat()} {cmd}\n')
        self.raw.flush()
        self.ser.write((cmd + '\r\n').encode())
        self.ser.flush()

    def command(self, cmd, timeout=8, expect_prompt=True, expect_regex=None):
        self.buf = ''
        self.send(cmd)
        deadline = time.monotonic() + timeout
        out = ''
        matcher = re.compile(expect_regex, re.I) if isinstance(expect_regex, str) else expect_regex
        for chunk in self._read(deadline):
            out += chunk
            if CRASH.search(out):
                break
            # Config commands print an explicit acknowledgement before the shell
            # prompt.  Prefer that over waiting for a prompt because asynchronous
            # Zephyr/W5500 logging can immediately follow the prompt and make an
            # end-anchored prompt detector miss it for the full timeout.
            if matcher is not None and matcher.search(out):
                break
            if expect_prompt and PROMPT.search(out[-200:]):
                break
        return out

    def bench(self, direction, timeout):
        self.buf = ''
        self.send(f'stcp bench {direction}')
        deadline = time.monotonic() + timeout
        out = ''
        linebuf = ''
        json_parts = []
        numbered_parts = {}
        in_json = False
        expected_len = None
        expected_parts = None
        pending_json = None
        json_end_at = None

        def handle_line(line):
            nonlocal in_json, expected_len, expected_parts, json_parts, numbered_parts
            m = JSON_BEGIN.search(line)
            if m:
                in_json = True
                expected_len = int(m.group(1))
                expected_parts = int(m.group(2)) if m.group(2) else None
                json_parts = []
                numbered_parts = {}
                return None

            m = JSON_PART.search(line)
            if in_json and m:
                idx, count, payload = m.groups()
                if idx is not None:
                    idx = int(idx); count = int(count)
                    if expected_parts is None:
                        expected_parts = count
                    numbered_parts[idx] = payload.strip()
                else:
                    json_parts.append(payload.strip())
                return None

            if in_json and JSON_END.search(line):
                in_json = False
                if numbered_parts:
                    missing = []
                    if expected_parts is not None:
                        missing = [i for i in range(expected_parts) if i not in numbered_parts]
                    payload = ''.join(numbered_parts[i] for i in sorted(numbered_parts))
                    if missing:
                        return {'parse_error': f'missing numbered JSON parts: {missing}',
                                'raw': payload, 'expected_len': expected_len,
                                'expected_parts': expected_parts,
                                'received_parts': sorted(numbered_parts)}
                else:
                    payload = ''.join(json_parts)
                try:
                    obj = json.loads(payload)
                except Exception as e:
                    obj = {'parse_error': str(e), 'raw': payload,
                           'expected_len': expected_len, 'expected_parts': expected_parts}
                if isinstance(obj, dict) and expected_len is not None and len(payload) != expected_len:
                    obj.setdefault('_wire_expected_len', expected_len)
                    obj.setdefault('_wire_actual_len', len(payload))
                return obj
            return None

        for chunk in self._read(deadline):
            out += chunk
            linebuf += chunk

            while True:
                nl_positions = [p for p in (linebuf.find('\n'), linebuf.find('\r')) if p >= 0]
                if not nl_positions:
                    break
                pos = min(nl_positions)
                line = linebuf[:pos]
                consume = pos + 1
                while consume < len(linebuf) and linebuf[consume] in '\r\n':
                    consume += 1
                linebuf = linebuf[consume:]
                obj = handle_line(line)
                if obj is not None:
                    # JSON_END is telemetry completion, not necessarily command
                    # completion. Keep it and wait for the transport's explicit
                    # success/failure marker. This is essential when Zephyr drops
                    # some JSON PART lines but the transfer itself succeeds.
                    pending_json = obj
                    json_end_at = time.monotonic()

            if CRASH.search(out):
                return out, pending_json, 'crash'
            if FAILURE.search(out):
                return out, pending_json, 'failure-text'
            if SUCCESS.search(out):
                return out, pending_json, 'success-text'

            # Some builds emit only JSON telemetry. After a short grace following
            # JSON_END, accept a complete JSON result as authoritative. For an
            # incomplete JSON block keep waiting for textual success/failure.
            if json_end_at is not None and time.monotonic() - json_end_at >= 1.0:
                if isinstance(pending_json, dict) and 'parse_error' not in pending_json:
                    return out, pending_json, 'json'

        if linebuf:
            obj = handle_line(linebuf)
            if obj is not None:
                pending_json = obj
        if SUCCESS.search(out):
            return out, pending_json, 'success-text'
        if FAILURE.search(out):
            return out, pending_json, 'failure-text'
        if pending_json is not None:
            return out, pending_json, 'json-timeout'
        return out, None, 'timeout'



def classify_bench(text, jr, how):
    """Classify transport result independently from optional telemetry integrity."""
    crash_m = CRASH.search(text)
    if crash_m:
        return 'crash', crash_m
    # Explicit benchmark failure always wins over telemetry/fallback success.
    if FAILURE.search(text):
        return 'fail', None
    # Complete JSON is authoritative when it parsed successfully.
    if isinstance(jr, dict) and 'parse_error' not in jr:
        st = jr.get('status')
        errs = jr.get('errors', 0)
        return ('pass' if (st in (0, '0', 'PASS', 'pass', None) and not errs) else 'fail'), None
    # UART/logging may drop STCP_BENCH_JSON_PART lines even though the actual
    # data transfer completed successfully.  That is a telemetry warning, not
    # an STCP transport failure.
    if SUCCESS.search(text):
        if isinstance(jr, dict) and 'parse_error' in jr:
            jr.setdefault('_telemetry_warning', jr.get('parse_error'))
        return 'pass', None
    if how == 'timeout':
        return 'timeout', None
    return 'fail', None


def run_hook(template: str | None, env: dict, log_path: Path):
    if not template:
        return 0
    cmd = template.format(**env)
    with log_path.open('a') as f:
        f.write(f'[{datetime.now(timezone.utc).isoformat()}] $ {cmd}\n')
        p = subprocess.run(cmd, shell=True, stdout=f, stderr=subprocess.STDOUT)
        f.write(f'rc={p.returncode}\n')
    return p.returncode


def choose_case(rng, phase, ports, chunks, min_total, max_total):
    if phase == 'full-pressure':
        direction = 'full'
        chunk = rng.choice(chunks[-max(1, len(chunks)//2):])
        total = rng.choice([max_total, max(min_total, max_total//2)])
    elif phase == 'tiny-churn':
        direction = rng.choice(['upload','download','full'])
        chunk = rng.choice(chunks[:max(1, len(chunks)//2)])
        total = rng.randint(min_total, min(max_total, min_total*8))
    else:
        direction = rng.choices(['upload','download','full'], weights=[30,30,40], k=1)[0]
        chunk = rng.choice(chunks)
        # Log-uniform-ish sizes without floats causing weird boundaries.
        candidates = [min_total, min_total*2, min_total*4, min_total*8, max_total//4, max_total//2, max_total]
        candidates = sorted(set(max(min_total, min(max_total, x)) for x in candidates))
        total = rng.choice(candidates)
    return direction, ports[direction], chunk, total


def parse_args():
    p = argparse.ArgumentParser(description='STCPv2 Zephyr real-world soak/stress orchestrator')
    p.add_argument('--serial', required=True, help='Zephyr console, e.g. /dev/ttyACM0')
    p.add_argument('--baud', type=int, default=115200)
    p.add_argument('--host', default='192.168.1.20')
    p.add_argument('--upload-port', type=int, default=19000)
    p.add_argument('--download-port', type=int, default=19001)
    p.add_argument('--full-port', type=int, default=19002)
    p.add_argument('--duration', type=int, default=8*3600, help='seconds, default 8h')
    p.add_argument('--case-timeout', type=int, default=90)
    p.add_argument('--min-total', type=int, default=64*1024)
    p.add_argument('--max-total', type=int, default=8*1024*1024)
    p.add_argument('--chunks', default='256,512,1024,1536,4096,8192,16384')
    p.add_argument('--seed', type=int, default=None)
    p.add_argument('--output', default=None)
    p.add_argument('--fault-every', type=int, default=0, help='run fault hook every N completed cases; 0=off')
    p.add_argument('--fault-command', default=os.getenv('STCP_FAULT_COMMAND'), help='shell command; format vars: seq,phase,direction')
    p.add_argument('--fault-recover-command', default=os.getenv('STCP_FAULT_RECOVER_COMMAND'))
    p.add_argument('--fault-down-seconds', type=float, default=3.0)
    p.add_argument('--continue-after-failure', action='store_true', default=True)
    p.add_argument('--stop-on-failure', dest='continue_after_failure', action='store_false')
    p.add_argument('--reconnect-burst', type=int, default=10, help='every cycle, number of small connection churn cases')
    return p.parse_args()


def main():
    a = parse_args()

    # One soak orchestrator per serial device.  Two readers/writers on the same
    # Zephyr console corrupt command/response ownership and can make an old run
    # appear to continue inside a new terminal session.
    lock_name = re.sub(r'[^A-Za-z0-9_.-]+', '_', os.path.realpath(a.serial))
    lock_path = Path(os.getenv('XDG_RUNTIME_DIR', '/tmp')) / f'stcp-soak-{lock_name}.lock'
    lock_fd = lock_path.open('w')
    try:
        fcntl.flock(lock_fd.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        print(f'ERROR: another STCP soak process already owns {a.serial} (lock {lock_path})', file=sys.stderr)
        print(f'Check with: pgrep -af stcp_realworld_soak.py', file=sys.stderr)
        return 3
    lock_fd.write(f'pid={os.getpid()} serial={a.serial} started={datetime.now(timezone.utc).isoformat()}\n')
    lock_fd.flush()
    seed = a.seed if a.seed is not None else int(time.time())
    rng = random.Random(seed)
    chunks = sorted({int(x) for x in a.chunks.split(',') if x.strip()})
    ports = {'upload': a.upload_port, 'download': a.download_port, 'full': a.full_port}
    stamp = datetime.now().strftime('%Y%m%d-%H%M%S')
    out = Path(a.output or f'stcp-realworld-{stamp}')
    out.mkdir(parents=True, exist_ok=True)
    raw_log = out/'zephyr-console.log'
    jsonl = out/'results.jsonl'
    events = out/'events.log'
    hooks = out/'fault-hooks.log'
    summary_path = out/'summary.json'
    meta = {
        'schema_version': 1, 'started_at': datetime.now(timezone.utc).isoformat(), 'seed': seed,
        'serial': a.serial, 'baud': a.baud, 'host': a.host, 'ports': ports,
        'duration_s': a.duration, 'case_timeout_s': a.case_timeout,
        'min_total': a.min_total, 'max_total': a.max_total, 'chunks': chunks,
        'fault_every': a.fault_every, 'fault_command': a.fault_command,
    }
    (out/'run.json').write_text(json.dumps(meta, indent=2) + '\n')

    stop = False
    def on_sig(sig, frame):
        nonlocal stop
        stop = True
    signal.signal(signal.SIGINT, on_sig)
    signal.signal(signal.SIGTERM, on_sig)

    shell = ZephyrShell(a.serial, a.baud, raw_log)
    stats = {'total':0,'pass':0,'fail':0,'timeout':0,'crash':0,'faults':0,'bytes_tx':0,'bytes_rx':0,'directions':{}}
    t0 = time.monotonic()
    seq = 0

    def event(msg):
        line = f'[{datetime.now(timezone.utc).isoformat()}] {msg}'
        print(line, flush=True)
        with events.open('a') as f: f.write(line+'\n')

    def setcfg(name, value):
        ack = {
            'transport': r'Transport\s*=\s*',
            'host':      r'Host\s*=\s*',
            'port':      r'Port\s*=\s*',
            'chunk':     r'Chunk\s*=\s*',
            'total':     r'Total\s*=\s*',
        }.get(name)
        outtxt = shell.command(f'stcp config {name} {value}', timeout=8,
                               expect_prompt=(ack is None), expect_regex=ack)
        if CRASH.search(outtxt):
            raise RuntimeError(f'crash while setting {name}')
        if ack and not re.search(ack, outtxt, re.I):
            raise RuntimeError(f'no acknowledgement while setting {name}={value}')

    def run_case(phase, idle_before_ms=0):
        nonlocal seq, stop
        seq += 1
        direction, port, chunk, total = choose_case(rng, phase, ports, chunks, a.min_total, a.max_total)
        if idle_before_ms:
            time.sleep(idle_before_ms/1000.0)
        event(f'CASE {seq} phase={phase} dir={direction} port={port} chunk={chunk} total={total}')
        try:
            setcfg('transport','stcp')
            setcfg('host', a.host)
            setcfg('port', port)
            setcfg('chunk', chunk)
            setcfg('total', total)
        except Exception as e:
            status='crash'; jr=None; text=str(e); elapsed=0.0
        else:
            s = time.monotonic()
            text, jr, how = shell.bench(direction, a.case_timeout)
            elapsed = time.monotonic()-s
            status, crash_m = classify_bench(text, jr, how)
        r = Result(seq, datetime.now(timezone.utc).isoformat(), phase, direction, port, chunk, total,
                   idle_before_ms, status, elapsed, jr, CRASH.search(text).group(0) if CRASH.search(text) else None)
        with jsonl.open('a') as f: f.write(json.dumps(asdict(r), separators=(',',':'))+'\n')
        stats['total'] += 1; stats[status] += 1
        ds = stats['directions'].setdefault(direction, {'total':0,'pass':0,'fail':0,'timeout':0,'crash':0})
        ds['total'] += 1; ds[status] += 1
        if isinstance(jr,dict):
            stats['bytes_tx'] += int(jr.get('bytes_tx') or 0)
            stats['bytes_rx'] += int(jr.get('bytes_rx') or 0)
        event(f'RESULT {seq} {status.upper()} elapsed={elapsed:.3f}s source={how if 'how' in locals() else 'config'} json_status={jr.get('status') if isinstance(jr, dict) else None} json_errors={jr.get('errors') if isinstance(jr, dict) else None} telemetry={'WARN' if isinstance(jr, dict) and jr.get('parse_error') else 'OK'}')
        if status in ('crash','timeout','fail') and not a.continue_after_failure:
            stop=True
        return status

    try:
        event(f'START seed={seed} duration={a.duration}s output={out}')
        shell.command('', timeout=3)
        event('CONFIGURE baseline')
        for k,v in [('transport','stcp'),('host',a.host)]:
            setcfg(k,v)

        # Warmup catches immediate regressions before the long randomized part.
        for direction in ('upload','download','full'):
            if stop: break
            seq += 1
            port=ports[direction]; chunk=4096; total=max(a.min_total, min(a.max_total, 1024*1024))
            event(f'WARMUP {seq} dir={direction}')
            setcfg('port',port); setcfg('chunk',chunk); setcfg('total',total)
            s=time.monotonic(); text,jr,how=shell.bench(direction,a.case_timeout); elapsed=time.monotonic()-s
            status, crash_m = classify_bench(text, jr, how)
            r=Result(seq,datetime.now(timezone.utc).isoformat(),'warmup',direction,port,chunk,total,0,status,elapsed,jr,crash_m.group(0) if crash_m else None)
            with jsonl.open('a') as f:f.write(json.dumps(asdict(r),separators=(',',':'))+'\n')
            stats['total']+=1; stats[status]+=1
            ds=stats['directions'].setdefault(direction,{'total':0,'pass':0,'fail':0,'timeout':0,'crash':0}); ds['total']+=1; ds[status]+=1
            if isinstance(jr,dict): stats['bytes_tx']+=int(jr.get('bytes_tx') or 0); stats['bytes_rx']+=int(jr.get('bytes_rx') or 0)
            event(f'WARMUP RESULT {status.upper()} elapsed={elapsed:.3f}s source={how} json_status={jr.get('status') if isinstance(jr, dict) else None} json_errors={jr.get('errors') if isinstance(jr, dict) else None} telemetry={'WARN' if isinstance(jr, dict) and jr.get('parse_error') else 'OK'}')

        cycle=0
        while not stop and time.monotonic()-t0 < a.duration:
            cycle += 1
            # Realistic traffic is not constant saturation. Mix active bursts and idle gaps.
            for _ in range(rng.randint(4,10)):
                if stop or time.monotonic()-t0 >= a.duration: break
                run_case('mixed', idle_before_ms=rng.choice([0,0,0,10,50,100,250,1000,3000]))

            # High-pressure full duplex section.
            for _ in range(rng.randint(2,5)):
                if stop or time.monotonic()-t0 >= a.duration: break
                run_case('full-pressure', idle_before_ms=rng.choice([0,10,50]))

            # Reconnect/handshake churn with short transfers.
            for _ in range(a.reconnect_burst):
                if stop or time.monotonic()-t0 >= a.duration: break
                run_case('tiny-churn', idle_before_ms=rng.choice([0,5,20,100]))

            # Simulate quiet IoT periods followed by immediate traffic.
            if not stop and time.monotonic()-t0 < a.duration:
                idle_ms=rng.randint(1000,10000)
                event(f'IDLE {idle_ms}ms')
                run_case('post-idle', idle_before_ms=idle_ms)

            # Optional external physical/network fault injection.
            if a.fault_every and seq and seq % a.fault_every < (4+5+a.reconnect_burst+1):
                env={'seq':seq,'phase':'fault','direction':'mixed'}
                event(f'FAULT inject seq={seq}')
                stats['faults'] += 1
                run_hook(a.fault_command, env, hooks)
                time.sleep(a.fault_down_seconds)
                run_hook(a.fault_recover_command, env, hooks)
                time.sleep(1.0)
                if not stop: run_case('post-fault', idle_before_ms=0)

    except Exception as e:
        event(f'FATAL orchestrator exception: {e!r}')
        stats['orchestrator_error']=repr(e)
    finally:
        try: shell.command('stcp config show', timeout=3)
        except Exception: pass
        shell.close()
        stats['elapsed_s']=time.monotonic()-t0
        stats['ended_at']=datetime.now(timezone.utc).isoformat()
        stats['seed']=seed
        stats['pass_rate']=stats['pass']/stats['total'] if stats['total'] else 0.0
        stats['mib_tx']=stats['bytes_tx']/(1024*1024)
        stats['mib_rx']=stats['bytes_rx']/(1024*1024)
        summary_path.write_text(json.dumps(stats, indent=2)+'\n')
        event(f"END cases={stats['total']} pass={stats['pass']} fail={stats['fail']} timeout={stats['timeout']} crash={stats['crash']} TX={stats['mib_tx']:.2f}MiB RX={stats['mib_rx']:.2f}MiB")
        print(f'RESULT_DIR={out}')
    return 0 if stats['fail']==0 and stats['timeout']==0 and stats['crash']==0 else 1

if __name__ == '__main__':
    raise SystemExit(main())
