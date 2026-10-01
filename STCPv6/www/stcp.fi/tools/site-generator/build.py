#!/usr/bin/env python3
import argparse, json, shutil
from pathlib import Path

def load(p):
    with p.open(encoding="utf-8") as f: return json.load(f)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--source", required=True, type=Path)
    ap.add_argument("--output", required=True, type=Path)
    ap.add_argument("--quality-gate", type=Path)
    ap.add_argument("--performance", type=Path)
    a=ap.parse_args()

    if a.quality_gate:
        q=load(a.quality_gate)
        if q.get("result")!="PASS":
            raise SystemExit("REFUSE: quality gate is not PASS")

    if a.output.exists(): shutil.rmtree(a.output)
    shutil.copytree(a.source,a.output,ignore=shutil.ignore_patterns(".git","__pycache__","*.pyc"))

    data=a.output/"assets/data"; data.mkdir(parents=True,exist_ok=True)
    if a.quality_gate: shutil.copy2(a.quality_gate,data/"quality-gate.json")
    if a.performance:
        p=load(a.performance)
        if p.get("status") not in (None,"PASS") and p.get("result")!="PASS":
            raise SystemExit("REFUSE: performance data is not PASS")
        shutil.copy2(a.performance,data/"performance.json")

    print(f"Generated pristine site: {a.output}")

if __name__=="__main__": main()
