#!/usr/bin/env python3
from pathlib import Path
import re
from bs4 import BeautifulSoup

ROOT = Path(__file__).resolve().parent
SITE = ROOT / 'stcp.fi'

MENU = [
    ('home', 'Home', '/'),
    ('benchmarks', 'Benchmarks', '/benchmarks/'),
    ('github', 'GitHub', 'https://github.com/Paxsudos-IT/STCP'),
]

STATIC_PAGES = {
    SITE / 'index.html': 'home',
    SITE / 'technology' / 'index.html': 'technology',
    SITE / 'downloads' / 'index.html': 'downloads',
    SITE / 'documentation' / 'index.html': 'documentation',
    SITE / 'contact' / 'index.html': 'contact',
}

COMMON_CSS = '''
.global-header{position:sticky;top:0;z-index:50;background:rgba(3,11,24,.96);border-bottom:1px solid var(--line);backdrop-filter:blur(10px)}
.global-inner{width:min(1240px,96vw);margin:auto;display:flex;align-items:center;justify-content:space-between;gap:18px;padding:10px 12px}
.site-brand{font-weight:900;font-size:19px;letter-spacing:.04em;text-decoration:none;color:#fff}
.site-nav{display:flex;gap:5px;flex-wrap:wrap}
.site-nav a{padding:7px 9px;border-radius:7px;text-decoration:none;color:var(--muted)}
.site-nav a:hover,.site-nav a.active{color:#fff;background:#10233a}
@media(max-width:600px){.global-inner{align-items:flex-start;flex-direction:column}}
'''.strip()


def nav(active: str) -> str:
    links = ''.join(
        f'<a class="{"active" if key == active else ""}" href="{url}">{label}</a>'
        for key, label, url in MENU
    )
    return (
        '<header class="global-header"><div class="global-inner">'
        '<a class="site-brand" href="/">STCPv2</a>'
        f'<nav class="site-nav">{links}</nav>'
        '</div></header>'
    )


def patch_static(path: Path, active: str) -> None:
    if not path.exists():
        return
    text = path.read_text(encoding='utf-8')
    # Replace the old site header with the exact same header used by benchmark pages.
    text, count = re.subn(
        r'<header class="header"><div class="inner"><a class="brand" href="/">STCPv2</a><nav>.*?</nav></div></header>',
        nav(active),
        text,
        count=1,
        flags=re.S,
    )
    if count == 0:
        text, count = re.subn(
            r'<header class="global-header">.*?</header>', nav(active), text,
            count=1, flags=re.S,
        )
    if count == 0:
        raise RuntimeError(f'Header not found in {path}')
    if COMMON_CSS not in text:
        text = text.replace('</style>', COMMON_CSS + '</style>', 1)
    path.write_text(BeautifulSoup(text, 'html.parser').prettify(formatter='html5') + '\n', encoding='utf-8')


def patch_benchmark_pages() -> None:
    bench = SITE / 'benchmarks'
    for path in bench.rglob('*.html'):
        text = path.read_text(encoding='utf-8')
        # Guard against any accidental Raspberry Pi target in the global menu.
        text = re.sub(
            r'(<a class="(?:active)?" href=")[^"]*raspberry-pi/?(?:index\.html)?(">Benchmarks</a>)',
            r'\1/benchmarks/\2', text,
        )
        # Rebuild the global site navigation to guarantee exact equality everywhere.
        text, count = re.subn(
            r'<header class="global-header">.*?</header>', nav('benchmarks'), text,
            count=1, flags=re.S,
        )
        if count:
            path.write_text(BeautifulSoup(text, 'html.parser').prettify(formatter='html5') + '\n', encoding='utf-8')


for page, active in STATIC_PAGES.items():
    patch_static(page, active)
patch_benchmark_pages()
print('[OK] Common site menu synchronized')
print('[OK] Benchmarks menu target: https://stcp.fi/benchmarks/')
