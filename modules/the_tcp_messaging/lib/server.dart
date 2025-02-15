import 'dart:io';
import 'dart:convert';
import 'dart:typed_data';
import 'package:utils/utils.dart' as util;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_simple_http/the_simple_http.dart' as SH;
import 'package:encrypt/encrypt.dart' as enc;

import 'package:the_secure_comm/the_secure_comm.dart' as scom;

typedef processMsgFunction  = String? Function(Socket theSock, String msgIn);

class TcpServer {
  String host;
  int port;
  processMsgFunction theMessageHandler;
  String theAesKey; // 16, 24 tai 32 bytee
  ServerSocket? socket;

  TcpServer(this.host, this.port, this.theAesKey, this.theMessageHandler) {
    logi.traceMe("Setting up server on $host:$port ($theAesKey) CB: $theMessageHandler");
  }

  Future<void> start() async {
    socket = await ServerSocket.bind(host, port);
    logi.traceMe('Palvelin käynnistetty: ${socket!.address.address}:$port');

    await for (var sck in socket!) {
      _handleConnection(sck);
    }
  }

  void _handleConnection(Socket socket) {
    logi.traceMe(
        'Connection from ${socket.remoteAddress.address}:${socket.remotePort}');
    socket.listen(
      (data) {
        String? rv = theMessageHandler(socket, String.fromCharCodes(data));
        if (rv != null) {
          int len = 512;
          if (len > rv!.length) {
            len = rv!.length;
          }
          logi.traceMe("Sending now data from message handler: // ${rv!.substring(0,len)} //");
          socket.write(rv);
        }
      },
      onDone: () {
        logi.traceMe(
            'Connection closed: ${socket.remoteAddress.address}:${socket.remotePort}');
      },
    );
  }

  Future<void> stop() async {
    await socket?.close();
    logi.traceMe('Palvelin suljettu.');
  }
}

String processMessageCallback(String theMsg) {
  int len = 512;
  if (len > theMsg.length) {
    len = theMsg.length;
  }

  logi.traceMe("Server got raw message: // ${theMsg.substring(0, len)} //");
  // List<int> rv = theMsg.asUint8List();
  return "Kiitän ja kuittaan.";
}
