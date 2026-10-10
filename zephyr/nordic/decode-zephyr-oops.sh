#!/usr/bin/env bash
# Zephyr Cortex-M fault decoder. Always use the ELF from the flashed build.
set -Eeuo pipefail
usage() {
    cat <<'HELP'
Käyttö:
  decode-zephyr-oops.sh <fault.log|-> [zephyr.elf|build-hakemisto]
  cat fault.log | decode-zephyr-oops.sh - build-lte

ELF: argumentti, ZEPHYR_ELF tai ./build/zephyr/zephyr.elf.
Työkalut: ADDR2LINE / OBJDUMP, PATH tai ZEPHYR_SDK_INSTALL_DIR.
SECURE_ELF: valinnainen TF-M:n secure ELF; osoitteet ratkaistaan myös siitä.

Esimerkki:
  ./decode-zephyr-oops.sh lte-fault.log build-lte | tee decoded-lte.log

Käytä täsmälleen kaatuneeseen firmwareen kuuluvaa ELF:iä.
HELP
}
if [[ ${1:-} == --help || ${1:-} == -h ]]; then usage; exit 0; fi
[[ $# -ge 1 && $# -le 2 ]] || { usage >&2; exit 1; }
input=$1
elf=${2:-${ZEPHYR_ELF:-build/zephyr/zephyr.elf}}
if [[ -d "$elf" ]]; then
    if [[ -f "$elf/zephyr/zephyr.elf" ]]; then elf="$elf/zephyr/zephyr.elf";
    else elf="$elf/zephyr.elf"; fi
fi
[[ -r "$elf" ]] || { echo "Virhe: ELF ei ole luettavissa: $elf" >&2; exit 1; }
[[ "$input" == - || -r "$input" ]] || { echo "Virhe: loki ei ole luettavissa: $input" >&2; exit 1; }
command -v python3 >/dev/null || { echo 'Virhe: python3 puuttuu' >&2; exit 1; }
find_tool() {
    local name=$1 candidate root
    for candidate in "arm-zephyr-eabi-$name" "arm-none-eabi-$name" "llvm-$name" "$name"; do
        if command -v "$candidate" >/dev/null 2>&1; then command -v "$candidate"; return; fi
    done
    for root in "${ZEPHYR_SDK_INSTALL_DIR:-}" /opt/zephyr-sdk-* "$HOME"/zephyr-sdk-*; do
        [[ -n "$root" ]] || continue
        candidate="$root/arm-zephyr-eabi/bin/arm-zephyr-eabi-$name"
        if [[ -x "$candidate" ]]; then printf '%s\n' "$candidate"; return; fi
    done
    return 1
}
addr2line=${ADDR2LINE:-$(find_tool addr2line)}
objdump=${OBJDUMP:-$(find_tool objdump || true)}
command -v "$addr2line" >/dev/null || { echo 'Virhe: addr2line puuttuu; aseta ADDR2LINE' >&2; exit 1; }
if [[ -n ${SECURE_ELF:-} && ! -r "$SECURE_ELF" ]]; then
    echo "Virhe: SECURE_ELF ei ole luettavissa: $SECURE_ELF" >&2; exit 1
fi
task_tmp=$(mktemp -d)
trap 'rm -rf "$task_tmp"' EXIT
if [[ "$input" == - ]]; then cat > "$task_tmp/fault.log"; else cp -- "$input" "$task_tmp/fault.log"; fi
python3 - "$task_tmp/fault.log" "$elf" "$addr2line" "$objdump" "${SECURE_ELF:-}" <<'PY'
import hashlib, pathlib, re, subprocess, sys
log_path, elf, addr2line, objdump, secure = sys.argv[1:]
log = pathlib.Path(log_path).read_text(errors='replace')
if not log.strip():
    sys.exit('Virhe: loki on tyhjä')
print('ZEPHYR FAULT DECODE')
print('ELF:', pathlib.Path(elf).resolve())
print('SHA256:', hashlib.sha256(pathlib.Path(elf).read_bytes()).hexdigest())
print('ADDR2LINE:', addr2line)
print('PC tulkitaan sellaisenaan; LR:stä poistetaan Cortex-M Thumb-bitti.')
print('LR-2 on paluuta edeltävä osoite, ei varma kutsukäskyn alku.')
print('Osoite 0x0 ei yksin todista NULL-viittauksen syytä.\n')
elfs = [('Zephyr', elf)] + ([('Secure/TF-M', secure)] if secure else [])
def run(args):
    p = subprocess.run(args, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if p.stdout.strip(): print(p.stdout.rstrip())
    if p.returncode: print('Työkalun paluukoodi:', p.returncode)
seen = set()
pcs = set()
found = False
# Named PC/LR plus fault addresses and registers. Do not treat timestamps,
# decimal benchmark counters, xPSR, or EXC_RETURN as program addresses.
reg = re.compile(r'\b(r(?:1[0-5]|[0-9])(?:/[A-Za-z0-9]+)?|pc|lr)\s*:\s*(0x[0-9a-fA-F]+)', re.I)
for line in log.splitlines():
    entries = [(m.group(1), int(m.group(2),16)) for m in reg.finditer(line)]
    if re.search(r'Faulting instruction address', line, re.I):
        m = re.search(r':\s*(0x[0-9a-fA-F]+)', line)
        if m: entries.append(('PC', int(m.group(1),16)))
    elif not entries and re.search(r'\bAddress\s*:',line,re.I):
        m = re.search(r'Address\s*:\s*(0x[0-9a-fA-F]+)',line,re.I)
        if m: entries.append(('fault address',int(m.group(1),16)))
    if not entries: continue
    if line in seen: continue
    seen.add(line); found=True
    print('\nLOG:',line)
    for label,address in entries:
        is_lr = label.lower() in ('lr','r14/lr','r14')
        is_pc = label.lower() in ('pc','r15/pc','r15')
        print(f'  {label}: 0x{address:08x}')
        if address == 0:
            print('  NULL / osoite nolla; ei symboliratkaisua.'); continue
        if address >= 0xffffffe0:
            print('  Cortex-M EXC_RETURN / erikoisarvo; ei koodiosoite.'); continue
        addresses = [address & ~1] if is_lr else [address]
        if is_lr and addresses[0] >= 2: addresses.append(addresses[0]-2)
        if is_pc: pcs.add(address)
        for target in addresses:
            for name, binary in elfs:
                print(f'  [{name}] 0x{target:08x}')
                run([addr2line,'-a','-f','-C','-i','-e',binary,hex(target)])
if not found:
    sys.exit('Virhe: Zephyrin PC/LR/rekisteri- tai fault-osoitteita ei löytynyt.')
if pcs and objdump:
    for pc in sorted(pcs):
        for name,binary in elfs:
            print(f'\nDISASSEMBLY [{name}] PC=0x{pc:08x}')
            # Thumb instructions have variable width; start earlier for context.
            run([objdump,'-d','-S',f'--start-address={max(0,pc-32)}',f'--stop-address={pc+32}',binary])
elif pcs:
    print('\nObjdump puuttuu; disassembly ohitettu. Aseta OBJDUMP.')
print('\nHuomio: keskeytyksen aikana current thread voi olla idle ilman että idle aiheutti vian.')
if not secure and 'SECURE FAULT' in log:
    print('Secure Fault: tarvittaessa anna saman buildin TF-M ELF muuttujalla SECURE_ELF.')
PY
