#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
ENV_FILE="${STCP_PUBLISH_ENV:-$ROOT/.stcp-publish.env}"
[[ -f "$ENV_FILE" ]] && source "$ENV_FILE"
: "${STCP_PUBLISH_HOST:?Set STCP_PUBLISH_HOST}"
: "${STCP_PUBLISH_USER:?Set STCP_PUBLISH_USER}"
: "${STCP_PUBLISH_PORT:=22}"
 : "${STCP_PUBLISH_SITE_ROOT:=/var/www/stcp.fi}"

# Accept both forms:
#   STCP_PUBLISH_SITE_ROOT=/var/www/.../stcp.fi
# and the user's convenient form:
#   STCP_PUBLISH_SITE_ROOT=/var/www/.../stcp.fi/benchmarks
# In the latter case publish benchmarks directly there and the homepage to its parent.
if [[ "${STCP_PUBLISH_SITE_ROOT%/}" == */benchmarks ]]; then
  STCP_PUBLISH_REMOTE_DIR="${STCP_PUBLISH_REMOTE_DIR:-${STCP_PUBLISH_SITE_ROOT%/}}"
  STCP_PUBLISH_HOME_ROOT="${STCP_PUBLISH_HOME_ROOT:-$(dirname "${STCP_PUBLISH_SITE_ROOT%/}") }"
  STCP_PUBLISH_HOME_ROOT="${STCP_PUBLISH_HOME_ROOT% }"
else
  STCP_PUBLISH_HOME_ROOT="${STCP_PUBLISH_HOME_ROOT:-${STCP_PUBLISH_SITE_ROOT%/}}"
  STCP_PUBLISH_REMOTE_DIR="${STCP_PUBLISH_REMOTE_DIR:-${STCP_PUBLISH_SITE_ROOT%/}/benchmarks}"
fi

"$ROOT/generate-site.sh"
SITE="$ROOT/stcp.fi"
BENCH="$SITE/benchmarks"

required=(
  index.html
  raspberry-pi/index.html
  raspberry-pi/tcp/index.html
  raspberry-pi/udp/index.html
  zephyr/index.html
  compare/index.html
  releases/index.html
  raw/index.html
  manifest.json
)
for f in "${required[@]}"; do
  [[ -s "$BENCH/$f" ]] || { echo "[FAIL] Missing benchmark file: $f" >&2; exit 1; }
done

# The benchmark landing page must be a benchmark page, never the site homepage.
grep -q '<title>STCPv2 benchmark results</title>' "$BENCH/index.html" || {
  echo '[FAIL] benchmark/index.html is not the benchmark landing page' >&2
  exit 1
}
for href in 'raspberry-pi/tcp/index.html' 'raspberry-pi/udp/index.html' 'zephyr/index.html' 'compare/index.html'; do
  grep -q "href=\"$href\"" "$BENCH/index.html" || {
    echo "[FAIL] benchmark landing page lacks link: $href" >&2
    exit 1
  }
done

# Validate only the actual navigation element. Words such as Technology or
# Documentation may legitimately appear in page content and must not fail publish.
for page in "$SITE/index.html" "$BENCH/index.html" "$BENCH/raspberry-pi/tcp/index.html" "$BENCH/raspberry-pi/udp/index.html" "$BENCH/zephyr/index.html" "$BENCH/compare/index.html"; do
  nav_html="$(python3 - "$page" <<'PY_NAV'
from html.parser import HTMLParser
from pathlib import Path
import sys

class NavParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.depth = 0
        self.out = []
    def handle_starttag(self, tag, attrs):
        if tag == 'nav':
            self.depth += 1
        elif self.depth:
            self.depth += 1
        if self.depth:
            attr_text = ' '.join(f'{k}={v!r}' for k, v in attrs)
            self.out.append(f'<{tag} {attr_text}>')
    def handle_endtag(self, tag):
        if self.depth:
            self.out.append(f'</{tag}>')
            self.depth -= 1
    def handle_data(self, data):
        if self.depth:
            self.out.append(data)

parser = NavParser()
parser.feed(Path(sys.argv[1]).read_text(encoding='utf-8'))
print(' '.join(parser.out))
PY_NAV
)"
  [[ "$nav_html" == *'Home'* ]] || { echo "[FAIL] Home missing from menu: $page" >&2; exit 1; }
  [[ "$nav_html" == *'Benchmarks'* ]] || { echo "[FAIL] Benchmarks missing from menu: $page" >&2; exit 1; }
  [[ "$nav_html" == *'github.com/Paxsudos-IT/STCP'* ]] || { echo "[FAIL] GitHub missing from menu: $page" >&2; exit 1; }
  if grep -qE '>Technology<|>Downloads<|>Documentation<|>Contact<' <<<"$nav_html"; then
    echo "[FAIL] Dead menu entry remains in navigation of $page" >&2
    exit 1
  fi
done

REMOTE="$STCP_PUBLISH_USER@$STCP_PUBLISH_HOST"
RSYNC_SSH="ssh -p $STCP_PUBLISH_PORT"
SSH=(ssh -p "$STCP_PUBLISH_PORT")

# Publish homepage independently.
"${SSH[@]}" "$REMOTE" "mkdir -p '$STCP_PUBLISH_HOME_ROOT' '$STCP_PUBLISH_REMOTE_DIR'"
rsync -a -e "$RSYNC_SSH" "$SITE/index.html" "$REMOTE:$STCP_PUBLISH_HOME_ROOT/index.html"

# Publish the benchmark tree directly. This intentionally avoids latest/release
# symlink ambiguity that previously caused /benchmarks/ to serve the homepage.
rsync -a --delete -e "$RSYNC_SSH" "$BENCH/" "$REMOTE:$STCP_PUBLISH_REMOTE_DIR/"

# Verify the exact remote files after upload.
"${SSH[@]}" "$REMOTE" "set -e
  test -s '$STCP_PUBLISH_REMOTE_DIR/index.html'
  grep -q '<title>STCPv2 benchmark results</title>' '$STCP_PUBLISH_REMOTE_DIR/index.html'
  test -s '$STCP_PUBLISH_REMOTE_DIR/raspberry-pi/tcp/index.html'
  test -s '$STCP_PUBLISH_REMOTE_DIR/raspberry-pi/udp/index.html'
  test -s '$STCP_PUBLISH_REMOTE_DIR/zephyr/index.html'
  test -s '$STCP_PUBLISH_REMOTE_DIR/compare/index.html'"

echo '[OK] Published homepage to https://stcp.fi/'
echo '[OK] Published benchmark landing page to https://stcp.fi/benchmarks/'
echo '[OK] Published Raspberry Pi TCP, Raspberry Pi UDP, Zephyr and comparison pages'
