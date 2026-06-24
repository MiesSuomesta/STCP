import 'dart:io';
import 'dart:convert';
import 'dart:typed_data';
import 'package:encrypt/encrypt.dart' as enc;

import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_secure_comm/the_secure_comm.dart' as scom;
import 'package:stcp/stcp.dart';

class TcpServer {
  String host;
  int port;
  processMsgFunction theMessageHandler;
  theHandshakeFunction theHandshakeHandler;
  theHandshakeAESGotFunction theAesGotCallback;
  theHandshakePublicKeyGotFunction thePublicKeyGotCallback;
  ServerSocket? socket;

  StcpCommon theSC = StcpCommon();
  late EllipticCodec theEC;

  TcpServer(
      this.host,
      this.port,
      this.theEC,
      this.theMessageHandler,
      this.theHandshakeHandler,
      this.theAesGotCallback,
      this.thePublicKeyGotCallback) {
    logi.traceMe("Setting up server on $host:$port");
    logi.traceMe(
        "MSCB: $theMessageHandler HSH: $theHandshakeHandler, AG: $theAesGotCallback");
    logi.traceMe("PKGOT: $thePublicKeyGotCallback");
  }

  Future<void> bind() async {
    socket = await ServerSocket.bind(host, port);
    logi.traceMe('Palvelin käynnistetty: ${socket!.address.address}:$port');
  }

  Future<void> start() async {
    await bind();
    await for (var sck in socket!) {
      _handleConnection(sck);
    }
  }

  Map<String, Uint8List> theSharedKeys = {};
  String getKeyStringFromSock(Socket theSock) {
    return "${theSock.address.host}:${theSock.port}";
  }

  Future<void> _handleConnection(Socket theSocket) async {
    bool gotAES = false;
    bool gotClientPublicKey = false;
    bool connectionOK = false;

    String sharedKeysKey = getKeyStringFromSock(theSocket);
    logi.traceMe('Connection from ${sharedKeysKey} .. ');
    // BUG: Client connects (creates new socket)..

    // let same client connect multiple times without
    // hassle.
    if (theSharedKeys.containsKey(sharedKeysKey)) {
      gotClientPublicKey = true;
    }

    if (!gotClientPublicKey) {
      if (theHandshakeHandler != null) {
        logi.traceMe("started STCP handshake.. sending own publickey....");
        theHandshakeHandler(theSocket);
        logi.traceMe("Done STCP handshake....");
      }
    }

    logi.traceMe("Starting to listen for incoming data....");

    theSocket.listen((theDataIn) {
      String theDataInString =
          logi.logAndGetStringFromBytes("[STCP] Client sent", theDataIn); // OK

      /*
       * Serveri on jo ekana lähettäny oman public keyn, vastineeksi
       * serverille pitää tulla clientin public key
       * 
       * Serveri laittaa yhteisen salaisuuden AES avaimeksi, HMAC SHA256-käsit-
       * telyllä.
       *
       */

      if (!gotClientPublicKey) {
        if (theDataIn.length != 65) {
          logi.traceMe("[STCP] =========================================");
          logi.traceMe("[STCP] ==== Handshake process, no valid data...");
          logi.traceMe("[STCP] =========================================");
          return;
        }

        logi.traceMe("[STCP] =========================================");
        logi.traceMe("[STCP] ==== Handshake process ......");
        logi.traceMe("[STCP] =========================================");

        Uint8List theSharedSecret =
            thePublicKeyGotCallback(theSocket, theDataIn);

        String theSharedSecretString = logi.logAndGetStringFromBytes(
            "Shared secret", theSharedSecret.toList()); // OK
        logi.traceMe("[STCP] Got shared secret.....");
        String myAesKey = theEC.deriveSharedKeyBasedAESKey(theSharedSecret);
        logi.traceMe("[STCP] Got my AES key: // $myAesKey //");
        Uint8List myKeyList =
            logi.logAndGetBytesFromString("the AEA key", myAesKey);
        logi.traceMe(
            "[STCP] Setting aes for $sharedKeysKey => $theSharedSecretString");
        theSharedKeys[sharedKeysKey] = theSharedSecret;
        logi.traceMe("[STCP] Got aes: $myAesKey, passing it to STCP layer ..");
        theAesGotCallback(theSocket, myKeyList);
        logi.traceMe("[STCP] =========================================");
        logi.traceMe("[STCP] ==== Handshake process ENDS ......");
        logi.traceMe("[STCP] =========================================");
        gotClientPublicKey = true;
        return;
      }

      /*
       * Lopuksi: Clientti hanskaa AES salattuna liikenteen välittämistä
       * STCP-kerrokselle.
       */
      Uint8List theSharedKey = theSharedKeys[sharedKeysKey]!;
      String myAesKey = theEC.deriveSharedKeyBasedAESKey(theSharedKey);
      logi.traceMe("[STCP] =========================================");
      logi.traceMe("[STCP] ==== AES Traffic process ......");
      logi.traceMe("[STCP] ==== AES theSharedKey set: $theSharedKey");
      logi.traceMe("[STCP] ==== AES key set: $myAesKey");
      logi.traceMe("[STCP] =========================================");
      logi.logAndGetStringFromBytes("the AES traffic", theDataIn.toList());
      Uint8List? rv = theMessageHandler(theSocket, theDataIn);
      if (rv != null) {
        String out =
            logi.logAndGetStringFromBytes("Send after handler call", rv);

        int len = 512;
        if (len > rv!.length) {
          len = rv!.length;
        }
        logi.traceMe("Sending now data from message handler...");
        send(theSocket, rv!);
      }
    }); // . listen
  }

  send(Socket theSck, Uint8List message) {
    logi.logAndGetStringFromBytes(
        "Sending to ${getKeyStringFromSock(theSck)} via socket $theSck",
        message);

//    Socket.connect(host, port).then((theSck) {
    theSck.add(message);
    theSck.close();
//    });
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
