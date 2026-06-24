// ignore_for_file: file_names, unused_local_variable, library_prefixes, avoid_relative_lib_imports, non_constant_identifier_names, depend_on_referenced_packages, prefer_interpolation_to_compose_strings

import 'dart:developer' as dev;
import 'dart:typed_data';
import 'package:stack_trace/stack_trace.dart' as ST;

bool DEBUG_ENABLED_MASTER_SWITCH = true;
bool DEBUG_ENABLED_TO_STDOUT = true;

List<String> logFilterDisable = [];
List<String> logFilterEnable = [];
void traceMe(dynamic msg, {int frame = 1}) {
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

  if (DEBUG_ENABLED_TO_STDOUT) {
    print(calledFrom + ": $msg");
  } else {
    dev.log(calledFrom + ": $msg");
  }
}

void traceMeIf(bool bEnabled, dynamic msg) {
  if (!DEBUG_ENABLED_MASTER_SWITCH) {
    return;
  }
  if (!bEnabled) return;
  ST.Trace trace = ST.Trace.current(1);
  String calledFrom = trace.frames.first.toString();
  if (DEBUG_ENABLED_TO_STDOUT) {
    print(calledFrom + ": $msg");
  } else {
    dev.log(calledFrom + ": $msg");
  }
}

String logAndGetStringFromBytes(String title, List<int> theData) {
  String tmp = String.fromCharCodes(theData);
  traceMe("$title: // len: ${theData.length} // $tmp //", frame: 2);
  return tmp;
}

Uint8List logAndGetBytesFromString(String title, String theData) {
  Uint8List tmp = Uint8List.fromList(theData.codeUnits.toList());
  traceMe("$title: // len: ${tmp.length} // $theData //");
  return tmp;
}

void traceMeBetter(dynamic msg) {
  if (!DEBUG_ENABLED_MASTER_SWITCH) {
    return;
  }
  ST.Trace trace = ST.Trace.current();
  String calledFrom = trace.frames.first.toString();
  if (DEBUG_ENABLED_TO_STDOUT) {
    traceMe(calledFrom + ": $msg\n $calledFrom: $trace");
  } else {
    dev.log(calledFrom + ": $msg", stackTrace: trace);
  }
}
