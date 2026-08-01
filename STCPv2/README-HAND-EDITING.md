# Hand-editable generated site

The benchmark generator now writes readable, indented HTML instead of one-line/minified documents.

## Files to edit

- `stcp.fi/index.html` — site landing page
- `stcp.fi/benchmarks/index.html` — benchmark landing page
- `stcp.fi/benchmarks/raspberry-pi/tcp/index.html`
- `stcp.fi/benchmarks/raspberry-pi/udp/index.html`
- `stcp.fi/benchmarks/zephyr/index.html`
- `stcp.fi/benchmarks/compare/index.html`
- `stcp.fi/benchmarks/assets/report.css` — shared benchmark styles
- `stcp.fi/benchmarks/assets/report.js` — shared benchmark behaviour

## Important

Running `bash generate-report.sh` regenerates the benchmark HTML files. Permanent changes should therefore be made in `generate-site.py`, `CSS`, or `JS`. Temporary production fixes can be made directly in the generated HTML/CSS/JS files.

The page-specific benchmark dataset remains in a clearly marked `window.PAGE_DATA` script block in each report page.
