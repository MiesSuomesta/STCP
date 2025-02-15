library stcp;

import 'dart:io';
import 'package:the_tcp_messaging/server.dart' as tcpServer;
import 'package:paxlog/paxlog.dart' as logi;

import './stcpCommon.dart';

class StcpServer {
  String host;
  int port;
  processMsgFunction theUnsecureMessageHandler;
  late StcpCommon theCommon;
  ServerSocket? socket;
  late tcpServer.TcpServer theUnsecureServer;

  String? theSecureMessageTransfer(Socket theSock, String msgIncomingCrypted) {
    logi.traceMe("[SERVER] incoming raw: ${msgIncomingCrypted.length} // $msgIncomingCrypted //");
    String decryptedMessage = theCommon.theSecureMessageTransferIncoming(msgIncomingCrypted);
    logi.traceMe("[SERVER] incoming dec: ${decryptedMessage.length} // $decryptedMessage //");

    // the unsecure call
    String? unsecureResponse = theUnsecureMessageHandler(decryptedMessage);

    logi.traceMe("[SERVER] outgoing unsec: ${unsecureResponse.length} // $unsecureResponse //");
    String encryptedOut = theCommon.theSecureMessageTransferOutgoing(unsecureResponse);
    logi.traceMe("[SERVER] outgoing   sec: ${encryptedOut.length} // $encryptedOut //");
    return encryptedOut;
  }
  
  StcpServer(this.host, this.port, String theAesKey, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon(theAesKey);
    theCommon.theAesKey = theAesKey.padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0").substring(0, STCP_AES_KEY_SIZE_IN_BYTES);
    theUnsecureServer = tcpServer.TcpServer(host, port, theAesKey, theSecureMessageTransfer);
    /*
      read() -> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) -> send()
    */

    logi.traceMe("Initialized StcpServer");
  }

  Future<void> start() async {
    await theUnsecureServer.start();
  }

  Future<void> stop() async {
    theUnsecureServer.stop();
    logi.traceMe('Server closed.');
  }


}