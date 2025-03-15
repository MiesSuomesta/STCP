library stcp;

import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:pointycastle/export.dart';
import 'package:pointycastle/pointycastle.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:utils/utils.dart' as utils;

import './stcpCommon.dart';

class StcpClient {
  String host;
  int port;
  processMsgFunction theUnsecureMessageHandler;
  late Uint8List? theAesKey;
  late Uint8List? theSharedSecret;
  late StcpCommon theCommon;
  late EllipticCodec theEC = EllipticCodec();
  late Socket theSocket;
  Stream<Uint8List>? theInputStream;

  void theKeyExchangeProcess(Socket theSock) async {
    logi.traceMe(
        'Connection from ${theSock.address.host}:${theSock.port} at handshake...');

    Uint8List publicKeyBytes = theEC.getPublicKeyAsBytes();
    utils.logAndGetStringFromBytes(
        "Client send: own public key", publicKeyBytes);
    logi.traceMe("[STCP] Sending public key .....");
    send_raw(publicKeyBytes);
    logi.traceMe("[STCP] Sent public key .....");
  }

  Future<void> setTheAesKey(Socket theSocket, Uint8List theAesKeyGot) async {
    logi.traceMe("Setting the AES key...............");
    theAesKey = theAesKeyGot;
    logi.traceIntList("the AES key set to", theAesKey);
  }

  Uint8List? thePublicKeyGot(Socket theSocket, Uint8List thePublicKey) {
    if (theEC != null) {
      ECPublicKey gotPubKey = theEC!.bytesToPublicKey(thePublicKey);
      return theEC.computeSharedSecretAsBytes(gotPubKey);
    } else {
      return null;
    }
  }

  Future<Uint8List?> theSecureMessageTransfer(
      Socket theSock, Uint8List msgIncomingCrypted) async {
    logi.traceIntList("MSG transfer/plain", msgIncomingCrypted);
    //String theAESpresharedkey = theAesKey;

    Uint8List decryptedMessage = theCommon.theSecureMessageTransferIncoming(
        msgIncomingCrypted, theAesKey!);

    logi.traceIntList("MSG transfer/decrypted in", decryptedMessage);

    // the unsecure call
    Uint8List? unsecureResponse =
        await theUnsecureMessageHandler(theSock, decryptedMessage);

    logi.traceIntList("MSG transfer/decrypted out", unsecureResponse!);
    Uint8List encryptedOut = theCommon.theSecureMessageTransferOutgoing(
        unsecureResponse, theAesKey!);
    logi.traceIntList("MSG transfer/crypted out", encryptedOut);
    return encryptedOut;
  }

  bool theSocketIsInitted = false;
  Future<bool> initialize_socket() async {
    if (theSocketIsInitted) return false;
    logi.traceMe("Initialising socket....");
    theSocket = await Socket.connect(host, port);
    theSocketIsInitted = true;
    return true;
  }

  StcpClient(this.host, this.port, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon();
    theAesKey = Uint8List.fromList([0]);

    logi.traceMe("Init of StcpClient: $host:$port");

    /*
      | <--- done at recv ---> |                | <--- done at send ---> |
      read() --> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) --> send()
    */

    logi.traceMe("Initialized StcpClient");
  }

  Future<void> connect() async {
    bool gotPublicKey = false;
    await initialize_socket();

    logi.traceMe("[STCP] Connecting to $host:$port .....");

    if (theEC == null) {
      logi.traceMe("Elliptic curve is null!");
      throw Exception("STCP Protocol init failure: Elliptic Curve is not set.");
      return;
    }

    logi.traceMe("[STCP] =========================================");
    logi.traceMe("[STCP] ==== Handshake process ......");
    logi.traceMe("[STCP] =========================================");
    // Listen for publickey
    StreamSubscription<Uint8List>? subscription;
    /*
      * Serveri lähettää ekana oman public keyn, vastineeksi
      * serverille pitää tulla clientin public key
      * 
      * Serveri laittaa yhteisen salaisuuden AES avaimeksi, HMAC SHA256-käsit-
      * telyllä.
      *
      */
    theInputStream = theSocket.asBroadcastStream();
    await for (var theDataIn in theInputStream!) {
      bool okLen = theDataIn.length == 65;
      logi.traceIntList(
          "[STCP] at top of stream handler, data got (ok: $okLen)", theDataIn);
      if (!gotPublicKey) {
        logi.traceIntList(
            "[STCP] Got public key possibly from server: $okLen", theDataIn);
        if (okLen) {
          logi.traceIntList(
              "[STCP] Got public key possibly from server", theDataIn);
          theSharedSecret = this.thePublicKeyGot(theSocket, theDataIn);
          if (theSharedSecret != null) {
            theAesKey = theEC!.deriveSharedKeyBasedAESKey(theSharedSecret!);
            if (theAesKey != null) {
              setTheAesKey(theSocket, theAesKey!);
              logi.traceIntList("[STCP] Got AES key from server", theAesKey);
              gotPublicKey = true;
              // send own key...
              logi.traceMe("2. Sending my public key....");
              this.theKeyExchangeProcess(theSocket);
              break;
            }
          }
        }
      }
    }
    logi.traceMe("[STCP] =========================================");
    logi.traceMe("[STCP] ==== AES Traffic process ......");
    logi.traceIntList("[STCP] ==== AES theSharedSecret set", theSharedSecret);
    logi.traceIntList("[STCP] ==== AES key set", theAesKey);
    logi.traceMe("[STCP] =========================================");
  }

  Future<void> stcp_send(Uint8List message) async {
    await initialize_socket();
    logi.traceIntList("the AES day at send", theAesKey);

    Uint8List? theBytesOut =
        theCommon.theSecureMessageTransferOutgoing(message, theAesKey!);
    logi.traceIntList("STCP/send", theBytesOut);
    send_raw(theBytesOut);
  }

  Future<Uint8List?> stcp_recv() async {
    await initialize_socket();
    logi.traceIntList("the AES key at recv", theAesKey);

    Uint8List? encryptedIn = await this.recv_raw();

    Uint8List? theBytesOut =
        theCommon.theSecureMessageTransferIncoming(encryptedIn!, theAesKey!);

    logi.traceIntList("STCP/recv", theBytesOut);
    return theBytesOut;
  }

  void send_raw(Uint8List message) async {
    await initialize_socket();

    logi.traceMe("Sending to $host:$port via socket $theSocket");

    String HN = theSocket.remoteAddress.host;
    int HP = theSocket.remotePort;

    logi.traceIntList("TCP/send to $HN:$HP", message);

    //Socket.connect(host, port).then((theSck) {
    theSocket.add(message);
    //  theSck.close();
    //});
  }

  Future<Uint8List?> recv_raw() async {
    // await initialize_socket();

    String HN = theSocket.remoteAddress.host;
    int HP = theSocket.remotePort;
    Uint8List? message = null;
    logi.traceMe("TCP/recv from target: $HN:$HP.....");
    if (theInputStream == null) {
      logi.traceMe("TCP/recv no stream....");
    }
    logi.traceMe("TCP/recv waiting for message.....");
    await for (var theDataIn in theInputStream!) {
      message = theDataIn;
      break;
    }
    logi.traceMe("TCP/recv received.....");
    logi.traceIntList("TCP/recv ${message?.length}", message);
    return message;
  }

  Future<void> stop() async {}
}
