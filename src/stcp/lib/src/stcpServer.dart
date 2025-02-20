library stcp;

import 'dart:io';
import 'dart:convert';
import 'dart:typed_data';
import 'package:the_tcp_messaging/server.dart' as tcpServer;
import 'package:paxlog/paxlog.dart' as logi;

import 'package:pointycastle/export.dart';
import 'package:pointycastle/pointycastle.dart';

import './stcpCommon.dart';

class StcpServer {
  String host;
  int port;
  Map<String, dynamic> theSockedMetadata = {};
  processMsgFunction theUnsecureMessageHandler;
  ServerSocket? socket;
  late tcpServer.TcpServer theUnsecureServer;
  late StcpCommon theCommon;
  late EllipticCodec theEC = EllipticCodec();

  String getKeyStringFromSock(Socket theSock) {
    return "${theSock.address.host}:${theSock.port}";
  }

  void set_socket_meta(Socket theSock, dynamic theMeta) {
    String key = getKeyStringFromSock(theSock);
    theSockedMetadata[key] = theMeta;
  }

  Map<String, dynamic> get_socket_meta(Socket theSock) {
    String key = getKeyStringFromSock(theSock);
    return theSockedMetadata[key] ?? {};
  }

  Uint8List? theSecureMessageTransferAES(
      Socket theSock, Uint8List msgIncomingCrypted) {
    logi.logAndGetStringFromBytes(
        "[SERVER] Secure AES transfer incoming", msgIncomingCrypted);
    Map<dynamic, dynamic> theSockMeta = get_socket_meta(theSock);
    Uint8List theAESpresharedkey = theSockMeta["AESkey"];
    String theAESpresharedkeyStr =
        logi.logAndGetStringFromBytes("Secure AES", theAESpresharedkey);
    Uint8List decryptedMessage = theCommon.theSecureMessageTransferIncoming(
        msgIncomingCrypted, theAESpresharedkeyStr);
    logi.traceMe(
        "[SERVER] incoming dec: ${decryptedMessage.length} // $decryptedMessage //");

    // the unsecure call
    Uint8List? unsecureResponse =
        theUnsecureMessageHandler(theSock, decryptedMessage);

    logi.traceMe(
        "[SERVER] outgoing unsec: ${unsecureResponse?.length} // $unsecureResponse //");
    Uint8List encryptedOut = theCommon.theSecureMessageTransferOutgoing(
        unsecureResponse!, theAESpresharedkeyStr);
    logi.traceMe(
        "[SERVER] outgoing   sec: ${encryptedOut.length} // $encryptedOut //");
    return encryptedOut;
  }

  Uint8List decryptAESKey(Uint8List encryptedKey, RSAPrivateKey privateKey) {
    final decryptor = OAEPEncoding(RSAEngine())
      ..init(false, PrivateKeyParameter<RSAPrivateKey>(privateKey));
    return decryptor.process(encryptedKey);
  }

  void theKeyExchangeProcess(Socket theSock) {
    logi.traceMe(
        'Connection from ${theSock.address.host}:${theSock.port} at handshake...');
    Map<String, dynamic>? tmpMap = get_socket_meta(theSock);
    bool needHandshake = true;
    if (tmpMap.isNotEmpty) {
      if (tmpMap.containsKey("AESkey")) {
        String tmpKey = tmpMap["AESkey"];
        if (tmpKey.isNotEmpty) {
          logi.traceMe("Already have done handshake...");
          needHandshake = false;
        }
      }
    }
    logi.traceMe("[STCP] Sending public key? $needHandshake ..");
    if (needHandshake) {
      logi.traceMe("[STCP] Sending public key .....");
      Uint8List publicKeyBytes = theEC.getPublicKeyAsBytes();
      theSock.add(publicKeyBytes);
      theSock.flush();
    }
  }

  Future<void> setTheAesKey(Socket theSocket, Uint8List theAesKeyGot) async {
    String tmpString =
        logi.logAndGetStringFromBytes("SetTheAes:", theAesKeyGot.toList());
    logi.traceMe("[STCP] Got AES key: $tmpString");
    logi.traceMe(
        "[STCP] For ${theSocket.address.host}:${theSocket.port} the AES key is: $tmpString");
    Map<String, dynamic> tmp = get_socket_meta(theSocket);
    tmp["AESkey"] = theAesKeyGot;
    set_socket_meta(theSocket, tmp);
  }

  Uint8List thePublicKeyGot(Socket theSocket, Uint8List thePublicKey) {
    ECPublicKey gotPubKey = theEC.bytesToPublicKey(thePublicKey);
    return theEC.computeSharedSecretAsBytes(gotPubKey);
  }

  Uint8List? theSecureMessageTransfer(
      Socket theSock, Uint8List msgIncomingCrypted) {
    logi.logAndGetStringFromBytes(
        "[SERVER] Secure transfer incoming", msgIncomingCrypted);

    Uint8List? theList =
        theSecureMessageTransferAES(theSock, msgIncomingCrypted);

    logi.logAndGetStringFromBytes(
        "[SERVER] Secure transfer Outgoing", theList!.toList());

    return theList;
  }

  StcpServer(
      this.host, this.port, String theAesKey, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon();

    /*  theCommon.theAesKey = theAesKey
        .padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0")
        .substring(0, STCP_AES_KEY_SIZE_IN_BYTES);*/

    theUnsecureServer = tcpServer.TcpServer(
        host,
        port,
        theEC,
        theSecureMessageTransfer,
        theKeyExchangeProcess,
        setTheAesKey,
        thePublicKeyGot);
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
