import 'dart:io';
import 'package:intl/intl.dart';
import 'package:path/path.dart' as pth;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:utils/utils.dart' as util;

String getHomeDirectory() {
  if (Platform.isWindows) {
    return '${Platform.environment['USERPROFILE']}\\'; // Windows
  } else if (Platform.isLinux || Platform.isMacOS) {
    return '${Platform.environment['HOME']}/'; // Linux ja macOS
  } else {
    throw UnsupportedError('Unsupported platform');
  }
}

class SharedPresistentData {
  Map<String, dynamic> theSharedMap = {};

  Directory? theDataDirectory;
  File? theDataFileObject;

  SharedPresistentData() {
    String directory = getHomeDirectory();

    theDataDirectory = Directory("$directory/.paxsudos/multiserver");
    theDataDirectory?.createSync(recursive: true);

    String fullpath =
        pth.join(theDataDirectory!.path, 'multiserver_settings.json');
    theDataFileObject = File(fullpath);

    logi.traceMe("Set file for datas: $theDataFileObject ..");
    load_all();
    logi.traceMe("Content loaded!");
  }

  void load_all() {
    if (theDataFileObject!.existsSync()) {
      String tmpJsn = theDataFileObject!.readAsStringSync();
      theSharedMap = util.get_json_object_from_string(tmpJsn);
    }
  }

  void debug_dump_all() {
    if (logi.DEBUG_ENABLED_MASTER_SWITCH) {
      String tmpJsn = util.do_pretty_json_from_object("  ", theSharedMap);
      logi.traceMe("SharedPersistentData: // $tmpJsn //");
    }
  }

  void save_all() {
    String tmpJsn = util.do_pretty_json_from_object("  ", theSharedMap);
    theDataFileObject!.writeAsStringSync(tmpJsn);
  }

  dynamic _param_to_string(dynamic paramIN) {
    dynamic out = paramIN;
    if (paramIN is DateTime) {
      out = DateFormat('dd-MM-yyyy kk:mm:ss').format(paramIN);
      logi.traceMe("[Shared convert] DATE Converted: $paramIN -> $out");
    }
    return out;
  }

  bool setData(String theKey, dynamic theValue) {
    bool rv = theSharedMap.containsKey(theKey);
    theSharedMap[theKey] = _param_to_string(theValue);
    save_all();
    return rv;
  }

  dynamic getData(String theKey, String theDefault) {
    if (theSharedMap.containsKey(theKey)) {
      return theSharedMap[theKey];
    }
    return theDefault;
  }

  dynamic removeData(String theKey) {
    dynamic rv = theSharedMap.remove(theKey);
    save_all();
    return rv;
  }
}
