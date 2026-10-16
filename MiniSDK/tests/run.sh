#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
python_bin="${PYTHON:-python3}"
case "${1:-}" in
  --all) suites=(python rust golang dart) ;;
  --python) suites=(python) ;;
  --rust) suites=(rust) ;;
  --golang|--go) suites=(golang) ;;
  --dart) suites=(dart) ;;
  *) echo "Usage: tests/run.sh --all|--python|--rust|--golang|--dart"; exit 2 ;;
esac
if (( $# != 1 )); then echo 'Exactly one selector is required' >&2; exit 2; fi
"$python_bin" -c 'import robot' || { echo 'Install tests/requirements.txt in a venv first.' >&2; exit 1; }
run_dir="$root/test-results/$(date -u +%Y%m%dT%H%M%SZ)-$$"
mkdir -p "$run_dir"
status=0
for suite in "${suites[@]}"; do
  # STCP integrated security gate v1
  # MiniSDK: security belongs to the paid SDK. bash /srv/stcp-project/SDK/security/run-security-gate.sh || exit $?
  "$python_bin" -m robot --outputdir "$run_dir/$suite" "$root/tests/robot/$suite.robot" || status=1
done
echo "Reports: $run_dir"
exit "$status"
