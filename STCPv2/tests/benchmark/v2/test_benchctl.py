#!/usr/bin/env python3
import json
import subprocess
import tempfile
import unittest
from pathlib import Path

HERE = Path(__file__).resolve().parent
BENCHCTL = HERE / "benchctl.py"


class BenchctlTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.root = Path(self.temp.name)
        self.matrix = self.root / "cases.tsv"
        self.matrix.write_text("case_id\tenabled\tmode\tport\tclients\tpayload_bytes\tpipeline\tduration_s\tdirection\ncase-a\t1\ttls\t1\t1\t64\t1\t1\techo\n")
        self.run = self.root / "run-1"
        subprocess.run(["python3", BENCHCTL, "init", "--run-dir", self.run, "--cases", self.matrix], check=True, stdout=subprocess.DEVNULL)

    def tearDown(self): self.temp.cleanup()

    def result(self, operations=1):
        return {"case_id":"case-a","mode":"tls","direction":"echo","clients":1,"payload_bytes":64,"pipeline":1,"elapsed_s":1.0,"operations":operations,"errors":0,"error_details":[],"bytes_tx":64,"bytes_rx":64,"tx_mib_s":1.0,"rx_mib_s":1.0,"combined_mib_s":2.0,"operations_s":1.0,"connect_mean_ms":1.0,"rtt_p50_ms":1.0,"rtt_p95_ms":1.0,"rtt_p99_ms":1.0,"client_cpu_percent":1.0,"max_rss_kib":1}

    def test_pass_can_be_reported_ingested_and_approved(self):
        (self.run / "cases" / "case-a.json").write_text(json.dumps(self.result()))
        subprocess.run(["python3", BENCHCTL, "validate", "--run-dir", self.run], check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["python3", BENCHCTL, "report", "--run-dir", self.run], check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["python3", BENCHCTL, "ingest", "--run-dir", self.run, "--database", self.root/"db.sqlite"], check=True, stdout=subprocess.DEVNULL)
        subprocess.run(["python3", BENCHCTL, "approve", "--run-dir", self.run], check=True, stdout=subprocess.DEVNULL)
        self.assertTrue((self.run / "publish-approved.json").is_file())
        subprocess.run(["sha256sum", "-c", "SHA256SUMS"], cwd=self.run, check=True, stdout=subprocess.DEVNULL)

    def test_zero_operations_fails_closed(self):
        (self.run / "cases" / "case-a.json").write_text(json.dumps(self.result(0)))
        completed = subprocess.run(["python3", BENCHCTL, "validate", "--run-dir", self.run], stdout=subprocess.DEVNULL)
        self.assertNotEqual(completed.returncode, 0)
        self.assertEqual(json.loads((self.run / "validation.json").read_text())["status"], "FAIL")


if __name__ == "__main__": unittest.main()
