import 'dart:io';
import 'dart:convert';
import 'dart:async';
import 'dart:typed_data';
import 'package:utils/utils.dart' as util;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_simple_http/the_simple_http.dart' as SH;
import 'package:encrypt/encrypt.dart' as enc;

import 'package:the_secure_comm/the_secure_comm.dart' as scom;
import './thePaxsudosMessage.dart' as pm;


typedef processMsgFunction = String Function(String msgIn);

class TcpClient {
  final String host;
  final int port;
  final processMsgFunction theMessageHandler;
  final String theAesKey; // 16, 24 tai 32 bytee

  Socket? _socket;

  TcpClient(this.host, this.port, this.theAesKey, this.theMessageHandler) {
    logi.traceMe("At constructor: Connecting .....");
  }

  Future<void> connect() async {
    try {
      logi.traceMe('connecting to server: $host:$port ....');
      _socket = await Socket.connect( host, port );
      logi.traceMe('connected to server: ${_socket!.remoteAddress.address}:$port .. socket: $_socket');
    } catch (err) {
      logi.traceMe("Had an error while connecting..");
    }
  }

  Stream<String> recvStream() {
    StreamController scont = StreamController<String>();
    
    Stream<String> myStream = scont.stream as Stream<String>;
    logi.traceMe("[Socket listen (Client)] Stratin listening.....");

    _socket?.listen((theData){
        String toStream = String.fromCharCodes(theData);
        logi.traceMe("[Socket listen (Client)], $toStream");
        scont.add(toStream);
    });
    return myStream;
  }

  Future<void> send(String message) async {
    logi.traceMe("Sending to $host:$port via socket $_socket");
    if (_socket == null) {
      logi.traceMe("Socket was null, reconnecting....");
      await connect();
      logi.traceMe("Socket was null, reconnected.. Socket: $_socket");
    }
    int len = 512;
    if (len > message.length) {
        len = message.length;
    }
    logi.traceMe("Sending data to sock: $_socket // ${message.substring(0,len)} //");
    _socket?.write(message);
  }

  Future<void> disconnect() async {
    await _socket?.close();
    logi.traceMe('Disconnected.');
  }
}

