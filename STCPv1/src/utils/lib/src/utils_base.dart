// ignore_for_file: file_names, unused_local_variable, library_prefixes, avoid_relative_lib_imports, non_constant_identifier_names, depend_on_referenced_packages, prefer_interpolation_to_compose_strings

import 'dart:convert';
import 'dart:typed_data';
import 'dart:math' as Math;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:utils/utils.dart' as util;
import 'package:crc32_checksum/crc32_checksum.dart' as Crc32;
import 'package:stack_trace/stack_trace.dart' as ST;
import 'package:latlong2/latlong.dart';

class theLatLngBounds {
  final LatLng southwest;
  final LatLng northeast;
  late LatLng center;
  theLatLngBounds({required this.southwest, required this.northeast}) {
    center = calculate_center();
  }

  factory theLatLngBounds.fromPoints(List<LatLng> points) {
    double minLat = double.infinity;
    double maxLat = double.negativeInfinity;
    double minLng = double.infinity;
    double maxLng = double.negativeInfinity;

    for (var point in points) {
      if (point.latitude < minLat) {
        minLat = point.latitude;
      }
      if (point.latitude > maxLat) {
        maxLat = point.latitude;
      }
      if (point.longitude < minLng) {
        minLng = point.longitude;
      }
      if (point.longitude > maxLng) {
        maxLng = point.longitude;
      }
    }

    return theLatLngBounds(
      southwest: LatLng(minLat, minLng),
      northeast: LatLng(maxLat, maxLng),
    );
  }

  LatLng calculate_center() {
    double lat = northeast.latitude - southwest.latitude;
    double lng = northeast.longitude - southwest.longitude;

    // Divide
    lat /= 2;
    lng /= 2;

    // Add
    lat += southwest.latitude;
    lng += southwest.longitude;

    return LatLng(lat, lng);
  }

  bool contains(LatLng point) {
    return (point.latitude >= southwest.latitude &&
        point.latitude <= northeast.latitude &&
        point.longitude >= southwest.longitude &&
        point.longitude <= northeast.longitude);
  }

  @override
  String toString() {
    return 'theLatLngBounds(southwest: $southwest, northeast: $northeast)';
  }
}

LatLng getRandomLatLng() {
  double lat = Math.Random().nextDouble() * 180;
  double lng = Math.Random().nextDouble() * 180;
  lat -= 90;
  lng -= 90;
  return LatLng(lat, lng);
}

double calculateZoomForBB(
    LatLng pA, LatLng pB, double paramWidth, double paramHeight) {
  double minLat = Math.min(pA.latitude, pB.latitude);
  double maxLat = Math.max(pA.latitude, pB.latitude);

  double minLon = Math.min(pA.longitude, pB.longitude);
  double maxLon = Math.max(pA.longitude, pB.longitude);

  double mapWidth = paramWidth;
  double mapHeight = paramHeight;

  double width = maxLon - minLon;
  double height = maxLat - minLat;

  //double zoom = Math.log2e(mapWidth / (width) * 360 / (2 * Math.pi));
  double zoom = Math.log(mapWidth / (width * 256 / 360)) / Math.log(2);
  return zoom - 2;
}

theLatLngBounds getBBofPoints(LatLng pA, LatLng pB) {
  return theLatLngBounds.fromPoints([pA, pB]);
}

List<List<double>> calculate_bounding_box(LatLng middle, double radius) {
  double latDegree = 111320;
  // HMMMM....
  double lngDegree = 111320 * Math.cos(middle.latitude * Math.pi / 180);

  double deltaLat = radius / latDegree;
  double deltaLng = radius / lngDegree;

  double minLat = middle.latitude - deltaLat;
  double maxLat = middle.latitude + deltaLat;
  double minLng = middle.longitude - deltaLng;
  double maxLng = middle.longitude + deltaLng;

  // LAN,LNG
  List<double> bottom = [minLng, minLat];
  List<double> top = [maxLng, maxLat];
  List<double> center = [middle.longitude, middle.latitude];

  List<List<double>> TBC = [top, bottom, center];
  //logi.traceMe(
  //    "Got TBC: Top ${top.toString()}, Bottom: ${bottom.toString()}, Center: ${center.toString()}");
  return TBC;
}

double calculateDistanceOfPoints(LatLng a, LatLng b) {
  // Euklidinen etäisyys
  return (a.latitude - b.latitude).abs() + (a.longitude - b.longitude).abs();
}

String logAndGetStringFromBytes(String strName, Uint8List data) {
  String out = String.fromCharCodes(data.toList());
  //logi.traceMe("$strName: $data => $out", frame: 2);
  return out;
}

Uint8List logAndGetBytesFromString(String strName, String data) {
  List<int> out = data.codeUnits;
  //logi.traceMe("$strName: $data => $out", frame: 2);
  return Uint8List.fromList(out);
}

void __internal_util_writeYaml(
    StringBuffer buffer, Map<dynamic, dynamic> data, int indent) {
  var spaces = ' ' * indent;
  data.forEach((key, value) {
    buffer.writeln('$spaces$key:');
    if (value is Map) {
      __internal_util_writeYaml(buffer, value, indent + 2);
    } else if (value is List) {
      for (var item in value) {
        buffer.writeln('$spaces  - ${item is Map ? '' : item}');
        if (item is Map) {
          __internal_util_writeYaml(buffer, item, indent + 4);
        }
      }
    } else {
      buffer.writeln('$spaces  $value');
    }
  });
}

dynamic? get_if_exists_in_map(
    dynamic paramKey, Map<dynamic, dynamic> paramObj) {
  if (true) {
    String tmp = util.do_pretty_yaml_string_from_object(paramObj);
    logi.traceMe("$paramKey :: // $tmp //");
  }

  if (paramKey.isEmpty) {
    logi.traceMe("paramKey empty.");
    return null;
  }

  if (paramObj == null) {
    logi.traceMe("paramObj: Null!");
    return null;
  }

  if (paramObj.isEmpty) {
    logi.traceMe("paramObj: empty.");
    return null;
  }

  if (paramObj.containsKey(paramKey)) {
    return paramObj[paramKey];
  } else {
    logi.traceMeIf(true, "No section for $paramKey found!");
  }
  return null;
}

String do_pretty_yaml_string_from_object(Map<dynamic, dynamic> data) {
  var buffer = StringBuffer();
  __internal_util_writeYaml(buffer, data, 0);
  return buffer.toString();
}

String jsonToYaml(dynamic json, [int indent = 0]) {
  var yaml = StringBuffer();
  var indentation = ' ' * indent;

  if (json is Map<String, dynamic>) {
    json.forEach((key, value) {
      yaml.writeln('$indentation$key: ${jsonToYaml(value, indent + 2)}');
    });
  } else if (json is List) {
    for (var item in json) {
      yaml.writeln('$indentation- ${jsonToYaml(item, indent + 2)}');
    }
  } else {
    yaml.write(json);
  }

  return yaml.toString().trim();
}

List<dynamic> update_list(List<dynamic> toUpdate, List<dynamic> newStuff) {
  newStuff.forEach((elem) {
    int atIndex = toUpdate.indexOf(elem);
    if (atIndex > -1) {
      toUpdate[atIndex] = elem;
    } else {
      toUpdate.add(elem);
    }
  });
  return toUpdate;
}

bool is_zero_coordinate(LatLng coord) {
  LatLng zero = LatLng(0, 0);
  logi.traceMe("is_zero_coordinate:");
  return check_is_same_coordinate(coord, zero);
}

bool check_is_same_coordinate(LatLng coordA, LatLng coordB) {
  bool latSame = coordA.latitude == coordB.latitude;
  bool lngSame = coordA.longitude == coordB.longitude;
  bool same = latSame && lngSame;
  logi.traceMe("Lat same: $latSame, Lng same: $lngSame => Same: $same");
  return same;
}

bool check_is_not_same_coordinate(LatLng coordA, LatLng coordB) {
  bool same = check_is_same_coordinate(coordA, coordB);
  logi.traceMe("==> notsame: ${!same}");
  return !same;
}

LatLng swapLatLong2LongLat(LatLng from) {
  LatLng to = LatLng(from.longitude, from.latitude);
  logi.traceMe("Swapped: $from -> $to");
  return to;
}

String get_checksum_of(String data) {
  var calc = Crc32.Crc32.calculate(data);
  List<int> tmpl = calc.toString().codeUnits;
  String beCalc = base64Encode(tmpl.reversed.toString().codeUnits);
  return beCalc;
}

String do_pretty_json_from_string(String paramIndent, String paramJsonString) {
  if (paramJsonString.isEmpty) {
    logi.traceMe("Json empty.");
    return "{}";
  }
  paramJsonString = prepare_json_string(paramJsonString);

  Map<String, dynamic> jsnObj = get_json_object_from_string(paramJsonString);

  var encoder = JsonEncoder.withIndent(paramIndent);
  var out = encoder.convert(jsnObj);
  return out;
}

String do_pretty_json_from_string_raw(
    String paramIndent, String paramJsonString) {
  if (paramJsonString.isEmpty) {
    logi.traceMe("Json empty.");
    return "{}";
  }

  Map<String, dynamic> jsnObj = get_json_object_from_string(paramJsonString);

  var encoder = JsonEncoder.withIndent(paramIndent);
  var out = encoder.convert(jsnObj);
  return out;
}

String prepare_json_string(String strIn) {
  strIn = strIn.replaceAll("'", '"');
  strIn = strIn.replaceAll("True", 'true');
  strIn = strIn.replaceAll("False", 'false');
  strIn = strIn.replaceAll("None", '');
  return strIn;
}

String get_string_from_start(String paramStr, int len) {
  int cutFrom = len;

  if (len > paramStr.length) {
    cutFrom = paramStr.length;
  }

  logi.traceMe("PL: ${paramStr.length}, CL: $len, CF: $cutFrom");

  return paramStr.substring(0, cutFrom);
}

String do_pretty_json_from_object(String paramIndent, dynamic jsnObj) {
  var encoder = JsonEncoder.withIndent(paramIndent);
  var out = encoder.convert(jsnObj);
  return out;
}

void dump_pretty_json_from_object(String strJsonName, dynamic jsnObj) {
  var encoder = JsonEncoder.withIndent("  ");
  var out = encoder.convert(jsnObj);
  logi.traceMe("Json of $strJsonName: // $out //", frame: 2);
}

dynamic get_json_object_from_string(String paramJsonString) {
  if (paramJsonString.isEmpty) {
    logi.traceMe("Json empty.");
    return {};
  }

  paramJsonString = prepare_json_string(paramJsonString);

  JsonDecoder decoder = const JsonDecoder();
  logi.traceMe("paramJsonString: ${paramJsonString}");
  final rv = decoder.convert(paramJsonString);
  logi.traceMe("Json type: ${rv.runtimeType}");
  return rv;
}

dynamic get_json_object_from_string_raw(String paramJsonString) {
  if (paramJsonString.isEmpty) {
    logi.traceMe("Json empty.");
    return {};
  }

  JsonDecoder decoder = const JsonDecoder();
  logi.traceMe("paramJsonString: ${paramJsonString}");
  final rv = decoder.convert(paramJsonString);
  logi.traceMe("Json type: ${rv.runtimeType}");
  return rv;
}

double get_degrees_from_nmea_string(String pNmeaStr) {
  int pointAt = pNmeaStr.indexOf(".");
  int txtlen = pNmeaStr.length;
  int minStart = pointAt - 2;
  String asteetStr = pNmeaStr.substring(0, minStart);
  String minuutitStr = pNmeaStr.substring(minStart, txtlen);
  double asteetDbl = double.parse(asteetStr);
  double minuutitDbl = double.parse(minuutitStr);
  double ret = asteetDbl + (minuutitDbl / 60.0);
  logi.traceMe("Nmea ${pNmeaStr} -> $asteetStr & $minuutitStr ");
  logi.traceMe("Nmea $asteetDbl + ($minuutitDbl / 60) => loc: $ret");

  return ret;
}

double get_degrees_from_exif_string(String coordinate, String direction) {
  logi.traceMe("$coordinate & $direction");

  coordinate = coordinate.replaceFirst("[", "");
  coordinate = coordinate.replaceFirst("]", "");
  final strArr = coordinate.split(",");
  List<double> dlist = [];

  double do_parse(String sIN) {
    double ret = 0;
    if (sIN.indexOf("/") > 0) {
      List<String> tmpS = sIN.split("/");
      List<double> tmpD = [
        double.parse(tmpS[0]),
        double.parse(tmpS[1]),
      ];
      ret = tmpD[0] / tmpD[1];
      //  logi.traceMe("Got double: ${tmpD[0]} / ${tmpD[1]} =  $ret");
    } else {
      ret = double.parse(sIN);
    }
    // logi.traceMe("Got double: $ret");
    return ret;
  }

  strArr.forEach((strItem) {
    //logi.traceMe("Parsing $strItem");
    double ret = do_parse(strItem);
    //logi.traceMe("Parsed $strItem => $ret");
    dlist.add(ret);
  });
  //logi.traceMe("Doubles: $dlist");

  final degrees = dlist.elementAt(0) as double;
  final minutes = dlist.elementAt(1) as double;
  final seconds = dlist.elementAt(2) as double;

  double ret = degrees.toDouble() +
      (minutes.toDouble() / 60) +
      (seconds.toDouble() / 3600);

  if (direction == 'S' || direction == 'W') {
    ret = -ret; // Muuta negatiiviseksi etelä- tai länsipituudella
  }
  return ret;
}
