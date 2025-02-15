import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

// Paxsudos tavarat
import 'package:utils/utils.dart' as util;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_simple_http/the_simple_http.dart' as SH;

typedef PaxsudosMultiserverClientFunc = bool Function(
    Socket clientConn, Uint8List data);

typedef thePaxudosHandleSockConnectCallback = void Function(Socket);

typedef thePaxudosHandleOnDataCallback = void Function(Socket, Uint8List);

String ZEROBYTE = String.fromCharCode(0);

class PaxsudosMessage {
  String msgData = "";
  String msgService = ""; // Service to route the message to
  String msgClass = ""; // Message class
  String msgPayload = ""; // The beef

  PaxsudosMessage();

  fromSlices(msgService, msgClass, msgPayload) {
    msgData = joinData(msgService, msgClass, msgPayload);
  }

  fromStringList(List<String> tmpList) {
    msgData = joinData(tmpList[0], tmpList[1], tmpList[2]);
  }

  void dump() {
    logi.traceMe("Message:");
    logi.traceMe("   Data   :  $msgData");
    logi.traceMe("   Service:  $msgService");
    logi.traceMe("   Class  :  $msgClass");
    logi.traceMe("   Payload:  $msgPayload");
  }

  static bool is_raw_a_message(Uint8List theData) {
    List<String> tmp = [];
    int count = theData
        .where((val) {
          return val == 0;
        })
        .toList()
        .length;
    bool rv = count >= 2;
    return rv;
  }

  bool fillFromRaw(Uint8List byteList) {
    if (is_raw_a_message(byteList)) {
      msgData = String.fromCharCodes(byteList);
      List<String> dataSet = msgData.split(ZEROBYTE);
      msgService = dataSet[0];
      msgClass = dataSet[1];
      msgPayload = dataSet.getRange(2, dataSet.length).join(ZEROBYTE);
      logi.traceMe("Data: $byteList -> $dataSet // $msgPayload");
      dump();
      return true;
    } else {
      logi.traceMe("Not PAXMESSAGE format?...");
    }
    return false;
  }

  String joinData(String msgService, String msgClass, String msgPayload) {
    msgData = "${msgService}${ZEROBYTE}${msgClass}${ZEROBYTE}${msgPayload}";
    return msgData;
  }

  List<String> splitData() {
    return msgData.split(ZEROBYTE);
  }

  Uint8List asUint8List() {
    return utf8.encode(this.msgData);
  }
}
