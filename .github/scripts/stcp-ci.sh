#!/usr/bin/env bash
set -Eeuo pipefail
mode="${1:?Usage: stcp-ci.sh validate|build|benchmark|overnight}"
repo="${GITHUB_WORKSPACE:-$(pwd)}"
cd "$repo"
if [[ -f kernel/module/Makefile ]]; then
    source_root="$repo"
elif [[ -f STCP/kernel/module/Makefile ]]; then
    source_root="$repo/STCP"
else
    echo '[FAIL] Expected kernel/module/Makefile (STCP repository root)' >&2
    exit 1
fi
if [[ "$mode" == validate ]]; then
    # Validate maintained orchestration, not copied kernel/toolchain trees.
    count=0
    while IFS= read -r -d '' file; do
        bash -n "$file"
        count=$((count+1))
    done < <(find "$source_root/scripts" "$source_root/kernel/module/scripts" -type f -name '*.sh' -print0)
    (( count > 0 ))
    python3 - "$source_root" <<'PY'
import ast, pathlib, sys
root=pathlib.Path(sys.argv[1])
count=0
for directory in [root/'MiniSDK', root/'tests', root/'scripts']:
    if not directory.is_dir(): continue
    for p in directory.rglob('*.py'):
        if any(x in {'.venv','target','build','__pycache__'} for x in p.parts): continue
        ast.parse(p.read_text(), filename=str(p)); count+=1
print(f'Python syntax: {count} files checked')
PY
    echo "Shell syntax: $count scripts checked"
    exit 0
fi
out="$repo/ci-artifacts"
mkdir -p "$out"
# All workflows use the same group; flock additionally serializes runner services.
command -v flock >/dev/null
exec 9>"${STCP_CI_LOCK:-/tmp/stcp-ci-hardware.lock}"
flock -w 21600 9
export PATH="$HOME/.cargo/bin:$PATH"
if [[ "$mode" == build ]]; then
    for command_name in make rustup cargo rustc llvm-ar ld.lld readelf modinfo tar; do
        command -v "$command_name" >/dev/null
    done
    export RUST_TOOLCHAIN="${RUST_TOOLCHAIN:-nightly}"
    rustup run "$RUST_TOOLCHAIN" rustc --version
    # Use prepared kernel trees; this job never installs/loads a module.
    host_tree="${X86_KDIR:-/lib/modules/$(uname -r)/build}"
    rpi_tree="${RPI_KDIR:-/srv/stcp-project/STCP/kernel/raspberry}"
    build_root="$(mktemp -d "${RUNNER_TEMP:-/tmp}/stcp-ci-build.XXXXXX")"
    trap 'rm -rf -- "$build_root"' EXIT
    for platform in host rpi; do
        tree="$host_tree"; [[ "$platform" == host ]] || tree="$rpi_tree"
        for required in Makefile Module.symvers; do
            [[ -f "$tree/$required" ]] || { echo "[FAIL] Prepared $platform kernel tree missing $tree/$required" >&2; exit 1; }
        done
        dest="$build_root/$platform"; mkdir -p "$dest" "$out/$platform"
        tar -C "$source_root/kernel/module" --exclude=target --exclude=.rust-objects -cf - . | tar -C "$dest" -xf -
        make -C "$dest" PLATFORM="$platform" KDIR="$tree" clean
        set +e
        make -C "$dest" PLATFORM="$platform" KDIR="$tree" \
            CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}" \
            RUST_TOOLCHAIN="$RUST_TOOLCHAIN" JOBS="${JOBS:-4}" module 2>&1 | tee "$out/$platform/build.log"
        rc=${PIPESTATUS[0]}
        set -e
        (( rc == 0 )) || exit "$rc"
        test -s "$dest/stcp.ko"
        readelf -h "$dest/stcp.ko" > "$out/$platform/elf-header.txt"
        if [[ "$platform" == host ]]; then
            grep -q 'Machine:.*X86-64' "$out/$platform/elf-header.txt"
        else
            grep -q 'Machine:.*AArch64' "$out/$platform/elf-header.txt"
        fi
        cp "$dest/stcp.ko" "$out/$platform/"
        modinfo "$dest/stcp.ko" > "$out/$platform/modinfo.txt"
        if [[ -f "$dest/Module.symvers" ]]; then cp "$dest/Module.symvers" "$out/$platform/"; fi
    done
    (cd "$out" && sha256sum host/stcp.ko rpi/stcp.ko > SHA256SUMS)
    exit 0
fi
[[ "$mode" == benchmark || "$mode" == overnight ]] || { echo 'Unknown mode' >&2; exit 2; }
project="${STCP_PROJECT_ROOT:-/srv/stcp-project}"
# Current full-run internals still use /srv/stcp-project explicitly.
[[ "$project" == /srv/stcp-project ]] || { echo '[FAIL] Current full-run requires /srv/stcp-project' >&2; exit 1; }
test -f "$project/settings.sh"
test -f "$project/scripts/do-full-run.sh"
test -f "$project/utilities/tools/site-generator/full_run_pipeline.py"
# Never silently benchmark an unrelated installed revision.
actual="$(git -C "$project/STCP" rev-parse HEAD)"
expected="$(git -C "$source_root" rev-parse HEAD)"
[[ "$actual" == "$expected" ]] || { echo "[FAIL] Prepared STCP checkout is $actual; workflow checkout is $expected. Sync /srv/stcp-project/STCP first." >&2; exit 1; }
if [[ -n "$(git -C "$project/STCP" status --porcelain --untracked-files=no)" ]]; then
    echo '[FAIL] Prepared STCP checkout has tracked modifications; commit/sync first.' >&2; exit 1
fi
# Project environment must already contain Robot, serial tools, SSH/sudo and SDK.
project_python="${STCP_CI_PYTHON:-$project/.venv/bin/python}"
if [[ ! -x "$project_python" ]]; then project_python="$(command -v python3)"; fi
export PATH="$(dirname "$project_python"):$PATH"
"$project_python" -c 'import robot'
python3 - "$project" "$out" "$expected" <<'PY'
import json,pathlib,subprocess,sys
root=pathlib.Path(sys.argv[1]); out=pathlib.Path(sys.argv[2])
revisions={'workflow_stcp':sys.argv[3]}
for name,p in [('project',root),('STCP',root/'STCP'),('SDK',root/'SDK')]:
    revisions[name]=subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip()
(out/'revisions.json').write_text(json.dumps(revisions,indent=2)+'\n')
PY
args=(--all)
if [[ "$mode" == benchmark ]]; then args+=(--phases host-raspberry); fi
case "${STCP_CI_SITE_MODE:-none}" in
    none) ;;
    preview) args+=(--preview) ;;
    publish) args+=(--publish) ;;
    *) echo '[FAIL] Invalid site mode' >&2; exit 2 ;;
esac
case "${STCP_CI_SKIP_COMPILE:-false}" in
    true) args+=(--skip-compile) ;;
    false) ;;
    *) echo '[FAIL] Invalid skip_compile boolean' >&2; exit 2 ;;
esac
cd "$project"
set +e
bash scripts/do-full-run.sh "${args[@]}" 2>&1 | tee "$out/full-run.log"
rc=${PIPESTATUS[0]}
set -e
python3 - "$project" "$out" <<'PY'
import pathlib,re,shutil,sys
root=pathlib.Path(sys.argv[1]);out=pathlib.Path(sys.argv[2])
text=(out/'full-run.log').read_text(errors='replace')
matches=re.findall(r'^\[RESULT\] (?:PASS|FAIL) (.+)$',text,re.M)
if not matches:
    print('No completed full-run result marker; pipeline log retained.');sys.exit(0)
p=pathlib.Path(matches[-1]).resolve()
if p.parent != (root/'SDK/full-run-results').resolve() or not (p/'run.json').is_file():
    raise SystemExit('Invalid full-run result location')
shutil.copytree(p,out/'full-run-results'/p.name)
print('Archived this invocation:',p)
PY
exit "$rc"
