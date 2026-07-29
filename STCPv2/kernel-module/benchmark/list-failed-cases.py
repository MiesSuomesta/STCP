#!/usr/bin/env python3
"""
list-failed-cases.py

Tarkistaa benchmark-caset samalla hyväksyntälogiikalla kuin run-all.sh:n
valid_existing_result():

  - tulos-JSON on olemassa ja ei ole tyhjä
  - JSON on kelvollinen objekti
  - mode ja transport vastaavat kind-arvoa
  - clients, payload_bytes ja pipeline vastaavat cases.tsv-riviä
  - errors == 0
  - operations > 0
  - error_details on null tai []
  - elapsed_s on vähintään duration * RESUME_MIN_DURATION_RATIO

Käyttö:
  python3 list-failed-cases.py RESULTS_DIR
  RESUME_MIN_DURATION_RATIO=0.90 python3 list-failed-cases.py RESULTS_DIR
  python3 list-failed-cases.py RESULTS_DIR --case stcp-udp-c4-p64-q8
  python3 list-failed-cases.py RESULTS_DIR --json
"""

from __future__ import annotations

import argparse
import json
import math
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Any


@dataclass(frozen=True)
class Case:
    kind: str
    clients: int
    payload: int
    pipeline: int
    duration: float

    @property
    def name(self) -> str:
        return f"{self.kind}-c{self.clients}-p{self.payload}-q{self.pipeline}"


@dataclass
class Failure:
    case: Case
    result_file: Path
    reasons: list[str]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Listaa tulokset, joita benchmarkin auto-resume ei hyväksy "
            "valmiiksi onnistuneiksi."
        )
    )
    parser.add_argument("result_dir", type=Path)
    parser.add_argument(
        "--case",
        help="Rajaa casen nimen osalla, esim. stcp-udp-c4-p64-q8.",
    )
    parser.add_argument(
        "--json",
        action="store_true",
        dest="json_output",
        help="Tulosta JSON-muodossa.",
    )
    parser.add_argument(
        "--ratio",
        type=float,
        default=float(os.environ.get("RESUME_MIN_DURATION_RATIO", "0.90")),
        help=(
            "Vaadittu elapsed_s / duration -suhde. "
            "Oletus RESUME_MIN_DURATION_RATIO tai 0.90."
        ),
    )
    return parser.parse_args()


def expected_mode(kind: str) -> str:
    mapping = {
        "tcp": "tcp",
        "tls": "tls",
        "udp": "udp",
        "stcp-tcp": "stcp",
        "stcp-udp": "stcp",
    }
    if kind not in mapping:
        raise ValueError(f"Tuntematon kind: {kind}")
    return mapping[kind]


def expected_transport(kind: str) -> str:
    mapping = {
        "tcp": "tcp",
        "tls": "tls",
        "udp": "udp",
        "stcp-tcp": "tcp",
        "stcp-udp": "udp",
    }
    if kind not in mapping:
        raise ValueError(f"Tuntematon kind: {kind}")
    return mapping[kind]


def result_path(result_dir: Path, case: Case) -> Path:
    if case.kind in {"tcp", "tls", "stcp-tcp"}:
        carrier = "tcp"
    elif case.kind in {"udp", "stcp-udp"}:
        carrier = "udp"
    else:
        carrier = "unknown"

    return result_dir / carrier / f"{case.name}.json"


def load_cases(manifest: Path) -> list[Case]:
    if not manifest.is_file():
        raise FileNotFoundError(f"cases.tsv puuttuu: {manifest}")

    cases: list[Case] = []

    for line_no, raw_line in enumerate(
        manifest.read_text(encoding="utf-8", errors="replace").splitlines(),
        1,
    ):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue

        # Bash-scripti kirjoittaa tab-erotellun tiedoston, mutta split()
        # hyväksyy myös vahingossa välilyönneiksi muuttuneen tulosteen.
        fields = re.split(r"\s+", line)

        if fields[0].lower() == "kind":
            continue

        if len(fields) < 5:
            raise ValueError(
                f"{manifest}:{line_no}: odotettiin vähintään 5 saraketta, "
                f"saatiin {len(fields)}"
            )

        kind, clients, payload, pipeline, duration = fields[:5]

        try:
            cases.append(
                Case(
                    kind=kind,
                    clients=int(clients),
                    payload=int(payload),
                    pipeline=int(pipeline),
                    duration=float(duration),
                )
            )
        except ValueError as exc:
            raise ValueError(
                f"{manifest}:{line_no}: virheellinen case-rivi: {raw_line}"
            ) from exc

    return cases


def integer_field(value: dict[str, Any], name: str, reasons: list[str]) -> int | None:
    raw = value.get(name)
    try:
        return int(raw)
    except (TypeError, ValueError):
        reasons.append(f"{name} ei ole kelvollinen kokonaisluku: {raw!r}")
        return None


def number_field(value: dict[str, Any], name: str, reasons: list[str]) -> float | None:
    raw = value.get(name)
    try:
        result = float(raw)
    except (TypeError, ValueError):
        reasons.append(f"{name} ei ole kelvollinen numero: {raw!r}")
        return None

    if not math.isfinite(result):
        reasons.append(f"{name} ei ole äärellinen numero: {raw!r}")
        return None

    return result


def validate_case(result_dir: Path, case: Case, ratio: float) -> Failure | None:
    path = result_path(result_dir, case)
    reasons: list[str] = []

    if not path.exists():
        reasons.append("Tulos-JSON puuttuu.")
        return Failure(case, path, reasons)

    try:
        if path.stat().st_size == 0:
            reasons.append("Tulos-JSON on tyhjä.")
            return Failure(case, path, reasons)
    except OSError as exc:
        reasons.append(f"Tiedoston tarkistus epäonnistui: {exc}")
        return Failure(case, path, reasons)

    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except OSError as exc:
        reasons.append(f"Tiedoston lukeminen epäonnistui: {exc}")
        return Failure(case, path, reasons)
    except UnicodeDecodeError as exc:
        reasons.append(f"Tiedosto ei ole kelvollista UTF-8:aa: {exc}")
        return Failure(case, path, reasons)
    except json.JSONDecodeError as exc:
        reasons.append(
            f"Virheellinen JSON rivillä {exc.lineno}, "
            f"sarakkeessa {exc.colno}: {exc.msg}"
        )
        return Failure(case, path, reasons)

    if not isinstance(value, dict):
        reasons.append(
            f"JSONin juurielementti ei ole objekti vaan {type(value).__name__}."
        )
        return Failure(case, path, reasons)

    wanted_mode = expected_mode(case.kind)
    actual_mode = value.get("mode")
    if actual_mode != wanted_mode:
        reasons.append(
            f"mode ei täsmää: saatiin {actual_mode!r}, odotettiin {wanted_mode!r}."
        )

    wanted_transport = expected_transport(case.kind)
    actual_transport = value.get("transport")
    if actual_transport != wanted_transport:
        reasons.append(
            "transport ei täsmää: "
            f"saatiin {actual_transport!r}, odotettiin {wanted_transport!r}."
        )

    clients = integer_field(value, "clients", reasons)
    if clients is not None and clients != case.clients:
        reasons.append(
            f"clients ei täsmää: saatiin {clients}, odotettiin {case.clients}."
        )

    payload = integer_field(value, "payload_bytes", reasons)
    if payload is not None and payload != case.payload:
        reasons.append(
            "payload_bytes ei täsmää: "
            f"saatiin {payload}, odotettiin {case.payload}."
        )

    pipeline = integer_field(value, "pipeline", reasons)
    if pipeline is not None and pipeline != case.pipeline:
        reasons.append(
            f"pipeline ei täsmää: saatiin {pipeline}, odotettiin {case.pipeline}."
        )

    errors = integer_field(value, "errors", reasons)
    if errors is not None and errors != 0:
        reasons.append(f"errors={errors}, hyväksytty arvo on 0.")

    operations = integer_field(value, "operations", reasons)
    if operations is not None and operations <= 0:
        reasons.append(f"operations={operations}, arvon pitää olla > 0.")

    error_details = value.get("error_details")
    if error_details not in (None, []):
        reasons.append(
            "error_details ei ole tyhjä: "
            + json.dumps(error_details, ensure_ascii=False)
        )

    elapsed = number_field(value, "elapsed_s", reasons)
    required = case.duration * ratio
    if elapsed is not None and elapsed < required:
        reasons.append(
            f"elapsed_s={elapsed:.6f} on liian lyhyt; "
            f"vaaditaan vähintään {required:.6f} s "
            f"({case.duration:g} × {ratio:g})."
        )

    return Failure(case, path, reasons) if reasons else None


def filter_cases(cases: list[Case], selector: str | None) -> list[Case]:
    if not selector:
        return cases

    needle = selector.lower()
    selected = [case for case in cases if needle in case.name.lower()]
    if not selected:
        raise SystemExit(f"Casea ei löytynyt valinnalla: {selector}")
    return selected


def print_human(
    failures: list[Failure],
    total: int,
    result_dir: Path,
    ratio: float,
) -> None:
    valid = total - len(failures)

    print(f"[INFO] Result set: {result_dir}")
    print(f"[INFO] Sama minimikestosuhde kuin benchmarkissa: {ratio:g}")
    print(f"[INFO] Tarkistettu: {total} | VALID={valid} | INVALID={len(failures)}")
    print()

    if not failures:
        print("[OK] Kaikki caset kelpaavat benchmarkin auto-resumelle.")
        return

    print(
        f"[FAIL] {len(failures)} casea ei kelpaa "
        "benchmarkin auto-resume-tarkistuksessa:"
    )
    print()

    for index, failure in enumerate(failures, 1):
        case = failure.case
        print("=" * 78)
        print(f"CASE #{index}: {case.name}")
        print("-" * 78)
        print(f"kind       : {case.kind}")
        print(f"clients    : {case.clients}")
        print(f"payload    : {case.payload} B")
        print(f"pipeline   : {case.pipeline}")
        print(f"duration   : {case.duration:g} s")
        print(f"result     : {failure.result_file}")
        print("reason     :")
        for reason_index, reason in enumerate(failure.reasons, 1):
            print(f"  {reason_index}. {reason}")
        print()


def print_json_output(
    failures: list[Failure],
    total: int,
    result_dir: Path,
    ratio: float,
) -> None:
    payload = {
        "result_dir": str(result_dir),
        "minimum_duration_ratio": ratio,
        "checked": total,
        "valid": total - len(failures),
        "invalid": len(failures),
        "failed_cases": [
            {
                "name": failure.case.name,
                "kind": failure.case.kind,
                "clients": failure.case.clients,
                "payload_bytes": failure.case.payload,
                "pipeline": failure.case.pipeline,
                "duration_s": failure.case.duration,
                "result_file": str(failure.result_file),
                "reasons": failure.reasons,
            }
            for failure in failures
        ],
    }
    print(json.dumps(payload, ensure_ascii=False, indent=2))


def main() -> int:
    args = parse_args()
    result_dir = args.result_dir.expanduser().resolve()

    if not result_dir.is_dir():
        print(f"[ERROR] Hakemistoa ei löydy: {result_dir}", file=sys.stderr)
        return 2

    if not math.isfinite(args.ratio) or args.ratio < 0:
        print(f"[ERROR] Virheellinen --ratio: {args.ratio}", file=sys.stderr)
        return 2

    try:
        cases = load_cases(result_dir / "cases.tsv")
        cases = filter_cases(cases, args.case)
    except (OSError, ValueError) as exc:
        print(f"[ERROR] {exc}", file=sys.stderr)
        return 2

    failures = [
        failure
        for case in cases
        if (failure := validate_case(result_dir, case, args.ratio)) is not None
    ]

    if args.json_output:
        print_json_output(failures, len(cases), result_dir, args.ratio)
    else:
        print_human(failures, len(cases), result_dir, args.ratio)

    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
