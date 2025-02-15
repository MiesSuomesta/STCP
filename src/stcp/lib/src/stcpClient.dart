library stcp;

import 'dart:io';
import 'dart:async';

import 'package:the_tcp_messaging/client.dart' as tcpClient;
import 'package:paxlog/paxlog.dart' as logi;

import './stcpCommon.dart';

class StcpClient {
  String host;
  int port;
  ServerSocket? socket;
  late StcpCommon theCommon;
  late tcpClient.TcpClient theUnsecureClient;

  StcpClient(this.host, this.port, String theAesKey) {
    theCommon = StcpCommon(theAesKey);
    logi.traceMe("Init of StcpClient: $host:$port (AES KEY: $theAesKey)");
    theUnsecureClient = tcpClient.TcpClient(host, port);
    /*
      | <--- done at recv ---> |                | <--- done at send ---> |
      read() --> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) --> send()
    */

    logi.traceMe("Initialized StcpClient");
  }

  send(String payload) {
    String crypted = theCommon.theSecureMessageTransferOutgoing(payload);
    theUnsecureClient.send(crypted);
  }

  Stream<String> recvStream() {
    StreamController scont = StreamController<String>();

    Stream<String> myStream = scont.stream as Stream<String>;
    Stream<String> myIncomingStream = theUnsecureClient.recvStream();
    logi.traceMe("[STCP] Socket listen (Client)] Starting listening.....");

    myIncomingStream.listen((theData) {
        String incomingStreamCrypted = theData;
        logi.traceMe("[STCP] Socket listen (Client) CRYPTED IN, $incomingStreamCrypted");
        String decrypted = theCommon.theSecureMessageTransferIncoming(incomingStreamCrypted);
        logi.traceMe("[STCP] Socket listen (Client) PLAIN  OUT, $decrypted");
        scont.add(decrypted);
    });
    
    return myStream;
  }

  Future<void> connect() async {
    logi.traceMe("[STCP] Connecting to $host:$port .....");
    await theUnsecureClient.connect();
    logi.traceMe("[STCP] Connected to $host:$port .....");
  }

  Future<void> stop() async {
  }


}