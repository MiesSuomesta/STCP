#!/usr/bin/env python3
from __future__ import annotations

import argparse
import shutil
from pathlib import Path

START = "<!-- STCP_PIPELINE_SUMMARY_START -->"
END = "<!-- STCP_PIPELINE_SUMMARY_END -->"

BLOCK = r'''<!-- STCP_PIPELINE_SUMMARY_START -->
<style id="stcp-pipeline-summary-style">
.stcp-pipeline-summary {
  margin: 1.5rem 0;
  padding: 1.25rem;
  border: 1px solid rgba(148,163,184,.35);
  border-radius: 14px;
  background: rgba(15,23,42,.92);
  color: #e2e8f0;
}
.stcp-pipeline-summary h2 { margin: 0 0 1rem; font-size: 1.35rem; }
.stcp-pipeline-summary-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit,minmax(150px,1fr));
  gap: .75rem;
}
.stcp-pipeline-summary-item {
  padding: .8rem;
  border-radius: 10px;
  background: rgba(17,24,39,.9);
}
.stcp-pipeline-summary-label {
  display: block;
  color: #94a3b8;
  font-size: .75rem;
  letter-spacing: .06em;
  text-transform: uppercase;
}
.stcp-pipeline-summary-value {
  display: block;
  margin-top: .25rem;
  font-weight: 700;
  overflow-wrap: anywhere;
}
.stcp-pipeline-summary-status-complete { color: #4ade80; }
.stcp-pipeline-summary-status-failed { color: #f87171; }
.stcp-pipeline-summary-status-partial,
.stcp-pipeline-summary-status-running { color: #facc15; }
</style>

<section class="stcp-pipeline-summary" id="stcp-pipeline-summary">
  <h2>Benchmark run summary</h2>
  <div class="stcp-pipeline-summary-grid">
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Run</span><span class="stcp-pipeline-summary-value" data-summary="run_id">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Started</span><span class="stcp-pipeline-summary-value" data-summary="started_at">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Finished</span><span class="stcp-pipeline-summary-value" data-summary="finished_at">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Total duration</span><span class="stcp-pipeline-summary-value" data-summary="total_wall_duration_human">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Configured per case</span><span class="stcp-pipeline-summary-value" data-summary="configured_case_duration_s">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Average per case</span><span class="stcp-pipeline-summary-value" data-summary="average_case_wall_duration_s">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Cases</span><span class="stcp-pipeline-summary-value" data-summary="case_count">–</span></div>
    <div class="stcp-pipeline-summary-item"><span class="stcp-pipeline-summary-label">Result</span><span class="stcp-pipeline-summary-value" data-summary="status">–</span></div>
  </div>
</section>

<script id="stcp-pipeline-summary-script">
(() => {
  const set = (key, value) => {
    const node = document.querySelector(`[data-summary="${key}"]`);
    if (node) node.textContent = value ?? '–';
  };

  const seconds = value => {
    const number = Number(value);
    if (!Number.isFinite(number)) return '–';
    return `${number.toFixed(2)} s`;
  };

  fetch('./pipeline-summary.json', {cache: 'no-store'})
    .then(response => {
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      return response.json();
    })
    .then(summary => {
      set('run_id', summary.run_id);
      set('started_at', summary.started_at ? new Date(summary.started_at).toLocaleString() : '–');
      set('finished_at', summary.finished_at ? new Date(summary.finished_at).toLocaleString() : '–');
      set('total_wall_duration_human', summary.total_wall_duration_human || '–');
      set('configured_case_duration_s', summary.configured_case_duration_s == null ? '–' : `${summary.configured_case_duration_s} s`);
      set('average_case_wall_duration_s', seconds(summary.average_case_wall_duration_s));
      set('case_count', `${summary.case_count_completed} / ${summary.case_count_expected}`);
      set('status', `${String(summary.status || 'unknown').toUpperCase()} · PASS ${summary.passed ?? 0} · FAIL ${summary.failed ?? 0}`);

      const statusNode = document.querySelector('[data-summary="status"]');
      if (statusNode) {
        statusNode.classList.add(`stcp-pipeline-summary-status-${summary.status || 'unknown'}`);
      }
    })
    .catch(error => {
      console.warn('Unable to load pipeline-summary.json', error);
      set('status', 'SUMMARY UNAVAILABLE');
    });
})();
</script>
<!-- STCP_PIPELINE_SUMMARY_END -->'''


def inject(html: str) -> str:
    if START in html and END in html:
        before, rest = html.split(START, 1)
        _, after = rest.split(END, 1)
        return before + BLOCK + after

    lower = html.lower()
    for closing_tag in ("</main>", "</body>", "</html>"):
        index = lower.rfind(closing_tag)
        if index >= 0:
            return html[:index] + BLOCK + "\n" + html[index:]

    return html + "\n" + BLOCK + "\n"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--result-dir", required=True)
    parser.add_argument("--site-dir", required=True)
    parser.add_argument("--index", default="index.html")
    args = parser.parse_args()

    result_dir = Path(args.result_dir).resolve()
    site_dir = Path(args.site_dir).resolve()
    source = result_dir / "pipeline-summary.json"
    target = site_dir / "pipeline-summary.json"
    index = site_dir / args.index

    if not source.is_file():
        raise SystemExit(f"Missing summary: {source}")
    if not site_dir.is_dir():
        raise SystemExit(f"Missing site directory: {site_dir}")
    if not index.is_file():
        raise SystemExit(f"Missing generated landing page: {index}")

    shutil.copy2(source, target)
    html = index.read_text(encoding="utf-8")
    index.write_text(inject(html), encoding="utf-8")

    print(target)
    print(index)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
