library stcp;

import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:pointycastle/export.dart';
import 'package:pointycastle/key_generators/rsa_key_generator.dart';
import 'package:pointycastle/pointycastle.dart';

import 'package:the_tcp_messaging/client.dart' as tcpClient;
import 'package:paxlog/paxlog.dart' as logi;

import './stcpCommon.dart';

class StcpClient {
  String host;
  int port;
  processMsgFunction theUnsecureMessageHandler;

  String theAESKey = "";
  late tcpClient.TcpClient theUnsecureClient;
  late StcpCommon theCommon;
  late EllipticCodec theEC = EllipticCodec();

  void theKeyExchangeProcess(Socket theSock) {
    logi.traceMe(
        'Connection from ${theSock.address.host}:${theSock.port} at handshake...');

    Uint8List publicKeyBytes = theEC.getPublicKeyAsBytes();
    logi.logAndGetStringFromBytes(
        "Client send: own public key", publicKeyBytes);
    logi.traceMe("[STCP] Sending public key .....");
    theSock.add(publicKeyBytes);
    theSock.flush();
  }

  Future<void> setTheAesKey(Socket theSocket, Uint8List theAesKeyGot) async {
    String tmpStr =
        logi.logAndGetStringFromBytes("Set the AES key to", theAesKeyGot);
    logi.traceMe("[STCP] Got AES key: $tmpStr");
    logi.traceMe("[STCP] For ${theSocket.address.host}:${theSocket.port} "
        " the AES key is: $tmpStr");
    theAESKey = tmpStr;
  }

  Uint8List thePublicKeyGot(Socket theSocket, Uint8List thePublicKey) {
    ECPublicKey gotPubKey = theEC.bytesToPublicKey(thePublicKey);
    return theEC.computeSharedSecretAsBytes(gotPubKey);
  }

  Uint8List? theSecureMessageTransfer(
      Socket theSock, Uint8List msgIncomingCrypted) {
    logi.logAndGetStringFromBytes(
        "[CLIENT] Secure transfer incoming", msgIncomingCrypted);
    String theAESpresharedkey = theAESKey;

    logi.logAndGetStringFromBytes("[Client]   secure in", msgIncomingCrypted);
    Uint8List decryptedMessage = theCommon.theSecureMessageTransferIncoming(
        msgIncomingCrypted, theAESpresharedkey);
    logi.logAndGetStringFromBytes(
        "[Client] unsecure for handler", decryptedMessage);

    // the unsecure call
    Uint8List? unsecureResponse =
        theUnsecureMessageHandler(theSock, decryptedMessage);

    logi.logAndGetStringFromBytes("[Client] unsecure out", unsecureResponse!);
    Uint8List encryptedOut = theCommon.theSecureMessageTransferOutgoing(
        unsecureResponse, theAESpresharedkey);
    logi.logAndGetStringFromBytes("[Client]   secure out", encryptedOut);
    return encryptedOut;
  }

  StcpClient(this.host, this.port, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon();

    logi.traceMe("Init of StcpClient: $host:$port");
    theUnsecureClient = tcpClient.TcpClient(
        host,
        port,
        theEC,
        theSecureMessageTransfer,
        theKeyExchangeProcess,
        setTheAesKey,
        thePublicKeyGot);
    /*
      | <--- done at recv ---> |                | <--- done at send ---> |
      read() --> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) --> send()
    */

    logi.traceMe("Initialized StcpClient");
  }

  send(String payload) {
    Uint8List theBytes =
        logi.logAndGetBytesFromString("Sending stuff plain", payload);
    Uint8List crypted =
        theCommon.theSecureMessageTransferOutgoing(theBytes, theAESKey);
    logi.logAndGetStringFromBytes("Sending stuff crypted", crypted);
    theUnsecureClient.send(crypted);
  }

  Future<void> connect() async {
    logi.traceMe("[STCP] Connecting to $host:$port .....");
    await theUnsecureClient.connect();
    logi.traceMe("[STCP] Connected to $host:$port .....");
  }

  Future<void> stop() async {}
}
