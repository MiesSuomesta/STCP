import 'dart:convert';
import 'dart:io';
import 'dart:math';

double? _asDouble(dynamic v) {
  if (v == null) return null;
  if (v is num) return v.toDouble();
  if (v is String) return double.tryParse(v);
  return null;
}

int? _asInt(dynamic v) {
  if (v == null) return null;
  if (v is int) return v;
  if (v is num) return v.toInt();
  if (v is String) return int.tryParse(v);
  return null;
}

String fmtPct(double v) {
  // v is fraction, e.g. 0.123 => 12.3%
  final p = v * 100.0;
  return "${p.toStringAsFixed(2)}%";
}

String fmtNum(double v, {int decimals = 2}) => v.toStringAsFixed(decimals);

String fmtBytesPerSec(double bps) {
  const k = 1024.0;
  const m = 1024.0 * 1024.0;
  const g = 1024.0 * 1024.0 * 1024.0;
  if (bps >= g) return "${fmtNum(bps / g)} GiB/s";
  if (bps >= m) return "${fmtNum(bps / m)} MiB/s";
  if (bps >= k) return "${fmtNum(bps / k)} KiB/s";
  return "${bps.toStringAsFixed(0)} B/s";
}

String fmtMs(double ms) => "${ms.toStringAsFixed(3)} ms";

double pctChange(double a, double b) {
  // change from a -> b as fraction (b-a)/a, guarded
  if (a == 0.0) return b == 0.0 ? 0.0 : double.infinity;
  return (b - a) / a;
}

Map<String, dynamic> readJsonFile(String path) {
  final text = File(path).readAsStringSync();
  final obj = jsonDecode(text);
  if (obj is! Map<String, dynamic>) {
    throw FormatException("Root JSON must be an object: $path");
  }
  return obj;
}

Map<String, dynamic>? _lat(Map<String, dynamic> run) {
  final v = run['lat_ms'];
  if (v is Map<String, dynamic>) return v;
  return null;
}

double? latVal(Map<String, dynamic> run, String key) {
  final l = _lat(run);
  if (l == null) return null;
  return _asDouble(l[key]);
}

void row(String label, String a, String b, String delta) {
  final l = label.padRight(18);
  final aa = a.padLeft(14);
  final bb = b.padLeft(14);
  final dd = delta.padLeft(14);
  stdout.writeln("$l$aa$bb$dd");
}

String deltaStr(double? a, double? b, {bool higherIsBetter = true}) {
  if (a == null || b == null) return "-";
  final d = pctChange(a, b);
  if (d.isInfinite) return "∞";
  // for latency, lower is better => invert sign presentation if desired
  final shown = higherIsBetter ? d : -d;
  final sign = shown >= 0 ? "+" : "";
  return "$sign${fmtPct(shown)}";
}

void main(List<String> args) {
  if (args.length < 2) {
    stderr.writeln("Usage: dart run compare_runs.dart <baseline.json> <candidate.json>");
    stderr.writeln("Example: dart run compare_runs.dart tcp.json stcp.json");
    exit(2);
  }

  final basePath = args[0];
  final candPath = args[1];

  final base = readJsonFile(basePath);
  final cand = readJsonFile(candPath);

  double? baseDur = _asDouble(base['duration_s']);
  double? candDur = _asDouble(cand['duration_s']);

  int baseConn = _asInt(base['connects']) ?? 0;
  int candConn = _asInt(cand['connects']) ?? 0;

  int baseConnErr = _asInt(base['connect_err']) ?? 0;
  int candConnErr = _asInt(cand['connect_err']) ?? 0;

  int baseOps = _asInt(base['ops']) ?? 0;
  int candOps = _asInt(cand['ops']) ?? 0;

  int baseOpErr = _asInt(base['op_err']) ?? 0;
  int candOpErr = _asInt(cand['op_err']) ?? 0;

  double? baseRps = _asDouble(base['rps']);
  double? candRps = _asDouble(cand['rps']);

  double? baseBps = _asDouble(base['throughput_bytes_per_s']);
  double? candBps = _asDouble(cand['throughput_bytes_per_s']);

  double? baseLatAvg = latVal(base, 'avg');
  double? candLatAvg = latVal(cand, 'avg');
  double? baseLatP50 = latVal(base, 'p50');
  double? candLatP50 = latVal(cand, 'p50');
  double? baseLatP95 = latVal(base, 'p95');
  double? candLatP95 = latVal(cand, 'p95');
  double? baseLatP99 = latVal(base, 'p99');
  double? candLatP99 = latVal(cand, 'p99');

  stdout.writeln("Baseline : $basePath");
  stdout.writeln("Candidate: $candPath");
  stdout.writeln("");

  // status / errors
  stdout.writeln("Status");
  stdout.writeln("  connects     : $baseConn -> $candConn");
  stdout.writeln("  connect_err  : $baseConnErr -> $candConnErr");
  stdout.writeln("  ops          : $baseOps -> $candOps");
  stdout.writeln("  op_err       : $baseOpErr -> $candOpErr");

  final baseFirstErr = base['first_error'];
  final candFirstErr = cand['first_error'];
  if (baseFirstErr != null || candFirstErr != null) {
    stdout.writeln("  first_error (baseline): ${baseFirstErr ?? '-'}");
    stdout.writeln("  first_error (candidate): ${candFirstErr ?? '-'}");
  }
  stdout.writeln("");

  // main table
  stdout.writeln("Metric".padRight(18) +
      "Baseline".padLeft(14) +
      "Candidate".padLeft(14) +
      "Δ".padLeft(14));
  stdout.writeln("-" * 60);

  row(
    "duration",
    baseDur == null ? "-" : "${baseDur.toStringAsFixed(2)} s",
    candDur == null ? "-" : "${candDur.toStringAsFixed(2)} s",
    deltaStr(baseDur, candDur, higherIsBetter: false),
  );

  row(
    "rps",
    baseRps == null ? "-" : baseRps.toStringAsFixed(1),
    candRps == null ? "-" : candRps.toStringAsFixed(1),
    deltaStr(baseRps, candRps, higherIsBetter: true),
  );

  row(
    "throughput",
    baseBps == null ? "-" : fmtBytesPerSec(baseBps),
    candBps == null ? "-" : fmtBytesPerSec(candBps),
    deltaStr(baseBps, candBps, higherIsBetter: true),
  );

  row(
    "lat avg",
    baseLatAvg == null ? "-" : fmtMs(baseLatAvg),
    candLatAvg == null ? "-" : fmtMs(candLatAvg),
    deltaStr(baseLatAvg, candLatAvg, higherIsBetter: false),
  );

  row(
    "lat p50",
    baseLatP50 == null ? "-" : fmtMs(baseLatP50),
    candLatP50 == null ? "-" : fmtMs(candLatP50),
    deltaStr(baseLatP50, candLatP50, higherIsBetter: false),
  );

  row(
    "lat p95",
    baseLatP95 == null ? "-" : fmtMs(baseLatP95),
    candLatP95 == null ? "-" : fmtMs(candLatP95),
    deltaStr(baseLatP95, candLatP95, higherIsBetter: false),
  );

  row(
    "lat p99",
    baseLatP99 == null ? "-" : fmtMs(baseLatP99),
    candLatP99 == null ? "-" : fmtMs(candLatP99),
    deltaStr(baseLatP99, candLatP99, higherIsBetter: false),
  );

  stdout.writeln("");
  stdout.writeln("Interpretation hints:");
  stdout.writeln("  - Positive Δ on rps/throughput is better.");
  stdout.writeln("  - Positive Δ on latency rows means latency improved (lower).");
  stdout.writeln("  - If errors differ, treat performance deltas as secondary.");
}

