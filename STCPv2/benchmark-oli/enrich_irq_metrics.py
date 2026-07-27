#!/usr/bin/env python3
"""Merge before/after Raspberry IRQ snapshots into one benchmark result JSON."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def delta(after: Any, before: Any) -> int:
    return max(0, int(after or 0) - int(before or 0))


def nested(source: dict[str, Any], *keys: str, default: Any = 0) -> Any:
    current: Any = source
    for key in keys:
        if not isinstance(current, dict):
            return default
        current = current.get(key, default)
    return current


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--result", required=True)
    parser.add_argument("--before", required=True)
    parser.add_argument("--after", required=True)
    args = parser.parse_args()

    result_path = Path(args.result)
    result = json.loads(result_path.read_text())
    before = json.loads(Path(args.before).read_text())
    after = json.loads(Path(args.after).read_text())

    operations = max(int(result.get("operations", 0)), 0)
    elapsed = max(float(result.get("elapsed_s", 0.0)), 0.0)
    total_mib = (
        float(result.get("tx_mib_s", 0.0))
        + float(result.get("rx_mib_s", 0.0))
    ) * elapsed

    irq_total = delta(
        nested(after, "interrupts", "total"),
        nested(before, "interrupts", "total"),
    )
    network_irq = delta(
        nested(after, "interrupts", "network_total"),
        nested(before, "interrupts", "network_total"),
    )

    softirq_names = [
        "HI", "TIMER", "NET_TX", "NET_RX", "BLOCK",
        "IRQ_POLL", "TASKLET", "SCHED", "HRTIMER", "RCU",
    ]
    softirq_delta = {
        name.lower(): delta(
            nested(after, "softirqs", name),
            nested(before, "softirqs", name),
        )
        for name in softirq_names
    }

    cpu_names = [
        "user", "nice", "system", "idle", "iowait",
        "irq", "softirq", "steal",
    ]
    cpu_delta = {
        name: delta(
            nested(after, "cpu_jiffies", name),
            nested(before, "cpu_jiffies", name),
        )
        for name in cpu_names
    }
    busy_jiffies = sum(
        cpu_delta[name]
        for name in ("user", "nice", "system", "irq", "softirq", "steal")
    )
    total_jiffies = busy_jiffies + cpu_delta["idle"] + cpu_delta["iowait"]

    per_kop = 1000.0 / operations if operations else 0.0
    per_mib = 1.0 / total_mib if total_mib else 0.0

    result.update({
        "server_irq_total": irq_total,
        "server_network_irq": network_irq,
        "server_net_rx_softirq": softirq_delta["net_rx"],
        "server_net_tx_softirq": softirq_delta["net_tx"],
        "server_timer_softirq": softirq_delta["timer"],
        "server_sched_softirq": softirq_delta["sched"],
        "server_cpu_busy_jiffies": busy_jiffies,
        "server_cpu_total_jiffies": total_jiffies,
        "server_cpu_busy_percent": (
            busy_jiffies / total_jiffies * 100.0 if total_jiffies else 0.0
        ),
        "server_network_irq_per_1k_ops": network_irq * per_kop,
        "server_net_rx_softirq_per_1k_ops": softirq_delta["net_rx"] * per_kop,
        "server_net_tx_softirq_per_1k_ops": softirq_delta["net_tx"] * per_kop,
        "server_kernel_network_events_per_1k_ops": (
            network_irq + softirq_delta["net_rx"] + softirq_delta["net_tx"]
        ) * per_kop,
        "server_network_irq_per_mib": network_irq * per_mib,
        "server_net_rx_softirq_per_mib": softirq_delta["net_rx"] * per_mib,
        "server_net_tx_softirq_per_mib": softirq_delta["net_tx"] * per_mib,
        "server_irq_measurement": {
            "source": "/proc/interrupts, /proc/softirqs and /proc/stat",
            "scope": "Raspberry Pi server",
            "network_irq_lines_before": nested(
                before, "interrupts", "network_lines", default={}
            ),
            "network_irq_lines_after": nested(
                after, "interrupts", "network_lines", default={}
            ),
            "softirq_delta": softirq_delta,
            "cpu_jiffies_delta": cpu_delta,
            "warning": (
                "Kernel activity proxy only; not a direct electrical power measurement."
            ),
        },
    })

    result_path.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({
        "result": str(result_path),
        "network_irq": network_irq,
        "net_rx_softirq": softirq_delta["net_rx"],
        "net_tx_softirq": softirq_delta["net_tx"],
        "events_per_1k_ops": result["server_kernel_network_events_per_1k_ops"],
    }))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
