// ignore_for_file: file_names, unused_local_variable, library_prefixes, avoid_relative_lib_imports, non_constant_identifier_names, depend_on_referenced_packages, prefer_interpolation_to_compose_strings

import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';
import 'dart:convert';
import 'dart:developer' as dev;
import 'package:stack_trace/stack_trace.dart' as ST;
import 'dart:typed_data';

bool DEBUG_ENABLED_MASTER_SWITCH = true;
bool DEBUG_ENABLED_TO_STDOUT = true;

// Try catch traces on ?
bool TRY_CATCH_ENABLED_MASTER_SWITCH = true;
bool TRY_CATCH_ENABLED_MASTER_STACK_TRACES = true;

List<String> logFilterDisable = [];
List<String> logFilterEnable = [];

void traceMeBIG(dynamic msg, {int frame = 4, int maxlen = 512}) {
  traceMe(
    ".---------------------------------------------------------->",
    frame: frame,
  );
  if (msg is List) {
    msg.forEach((val) {
      traceMe("| $val", frame: frame, maxlen: maxlen);
    });
  } else {
    traceMe("| $msg", frame: frame, maxlen: maxlen);
  }
  traceMe("'-------------------------------------->", frame: frame);
}

void traceMe(dynamic msg, {int frame = 1, int maxlen = 512}) {
  if (!DEBUG_ENABLED_MASTER_SWITCH) {
    return;
  }
  ST.Trace trace = ST.Trace.current(frame);
  String calledFrom = trace.frames.first.toString();
  bool filterDisable = false;
  bool filterEnable = false;
  bool filter = false;

  logFilterEnable.forEach((filterStr) {
    filter = true;
    filterEnable |= calledFrom.contains(filterStr);
  });

  logFilterDisable.forEach((filterStr) {
    filter = true;
    filterDisable |= calledFrom.contains(filterStr);
  });

  if (filter) {
    if (filterEnable) {
      filter = false;
    } else {
      filter = filterDisable;
    }
  }

  if (filter) {
    return;
  }

  String out = "$calledFrom: $msg";

  if (maxlen > 0) {
    if (out.length > maxlen) {
      out = out.substring(0, maxlen);
    }
  }

  if (DEBUG_ENABLED_TO_STDOUT) {
    print(out);
  } else {
    dev.log(out);
  }
}

void traceMeIf(bool bEnabled, dynamic msg, {int frame = 1, int maxlen = 512}) {
  if (!DEBUG_ENABLED_MASTER_SWITCH) {
    return;
  }
  if (!bEnabled) return;
  ST.Trace trace = ST.Trace.current(frame);
  String calledFrom = trace.frames.first.toString();

  String out = "$calledFrom: $msg";
  if (out.length > maxlen) {
    out = out.substring(0, maxlen);
  }

  if (DEBUG_ENABLED_TO_STDOUT) {
    print(out);
  } else {
    dev.log(out);
  }
}

void traceMeBetter(dynamic msg, {int frame = 1, int maxlen = 512}) {
  if (!DEBUG_ENABLED_MASTER_SWITCH) {
    return;
  }
  ST.Trace trace = ST.Trace.current(frame);
  String calledFrom = trace.frames.first.toString();

  String out = "$calledFrom: $msg\n $calledFrom: $trace";
  if (out.length > maxlen) {
    out = out.substring(0, maxlen);
  }

  if (DEBUG_ENABLED_TO_STDOUT) {
    print(out);
  } else {
    dev.log(out);
  }
}

void traceTryCatch(
  Object err, {
  Object? theStack,
  int frame = 2,
  int maxlen = 512,
}) {
  if (!TRY_CATCH_ENABLED_MASTER_SWITCH) {
    return;
  }

  ST.Trace trace = ST.Trace.current(frame);
  String calledFrom = trace.frames.isNotEmpty
      ? trace.frames.first.toString()
      : "<no frame>";

  String out = "$calledFrom: TryCatch error: $err\n$calledFrom: $trace";
  if (out.length > maxlen) {
    out = out.substring(0, maxlen);
  }

  print(out);
  dev.log(out);

  if (TRY_CATCH_ENABLED_MASTER_STACK_TRACES) {
    if (theStack != null) {
      print("Stack:\n$theStack");
      dev.log("Stack:\n$theStack");
    }
  }
}

dynamic unsetHash(Map<String, dynamic> pMap, List<String> pKeys) {
  Map<String, dynamic> out = Map.from(pMap);
  pKeys.forEach((key) {
    out.remove(key);
  });
  return out;
}

void traceIntList(
  String sName,
  Uint8List? dataIn, {
  int nLenMax = 512,
  bool noStr = false,
  int frame = 2,
}) {
  if (dataIn == null) {
    traceMe(
      "${sName}: no data =============================================================",
      frame: frame,
    );
    return;
  }

  int len = nLenMax;
  if (len > dataIn.length) {
    len = dataIn.length;
  }

  traceMe(
    "${sName} data len ${dataIn.length} ==================",
    frame: frame,
  );
  traceMe("${sName} raw  : ${dataIn.sublist(0, len)}", frame: frame);

  if (noStr) return;

  String tmp = String.fromCharCodes(dataIn.toList());
  traceMe("${sName} str len ${tmp.length} ==================", frame: frame);
  if (tmp.isNotEmpty) {
    if (tmp.length >= len) {
      traceMe("${sName} str  : ${tmp.substring(0, len)}", frame: frame);
    } else {
      traceMe("${sName} str  : $tmp", frame: frame);
    }
  }
}

void traceIntListIf(
  bool cond,
  String sName,
  Uint8List? dataIn, {
  int nLenMax = 512,
  bool noStr = false,
}) {
  if (cond) {
    traceIntList(sName, dataIn, noStr: noStr, nLenMax: nLenMax, frame: 3);
  }
}
