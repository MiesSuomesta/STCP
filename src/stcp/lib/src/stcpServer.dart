library stcp;

import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:the_tcp_messaging/server.dart' as tcpServer;
import 'package:paxlog/paxlog.dart' as logi;

import 'package:pointycastle/export.dart';
import 'package:pointycastle/pointycastle.dart';
import 'package:utils/utils.dart' as utils;

import './stcpCommon.dart';

class StcpServer {
  String host;
  int port;
  Map<String, dynamic> theSockedMetadata = {};
  processMsgFunction theUnsecureMessageHandler;
  late ServerSocket socket;
  // late tcpServer.TcpServer theUnsecureServer;
  late StcpCommon theCommon;
  late EllipticCodec theEC = EllipticCodec();

  void set_socket_meta(Socket theSock, dynamic theMeta) {
    String key = getKeyStringFromSock(theSock);
    theSockedMetadata[key] = theMeta;
    //utils.dump_pretty_json_from_object("Socket Meta (SET)", theSockedMetadata);
  }

  Map<String, dynamic> get_socket_meta(Socket theSock) {
    String key = getKeyStringFromSock(theSock);
    //utils.dump_pretty_json_from_object("Socket Meta (GET)", theSockedMetadata);
    return theSockedMetadata[key] ?? {};
  }

  Future<Uint8List?> theSecureMessageTransferAES(Socket theSock,
      Uint8List msgIncomingCrypted, Uint8List? theAESDerivedKey) async {
    logi.traceIntList("Secure message transfer AES input", msgIncomingCrypted);
    logi.traceIntList("The AES key", theAESDerivedKey);
    if (theAESDerivedKey == null) {
      throw Exception(
          "STCP Protocol init failure: AES secret is not set to metadata!");
    }

    Uint8List decryptedMessage = theCommon.theSecureMessageTransferIncoming(
        msgIncomingCrypted, theAESDerivedKey!);

    logi.traceIntList("[SERVER] AES message decrypted", decryptedMessage);

    // the unsecure call
    logi.traceMe("Undesure callback call ===================================");

    Uint8List? unsecureResponse =
        await theUnsecureMessageHandler(theSock, decryptedMessage);

    logi.traceIntList("[SERVER] UNSEC returned", unsecureResponse);
    logi.traceMe("Undesure callback done ===================================");

    logi.traceIntList("[SERVER] outgoing", unsecureResponse);

    if (unsecureResponse != null) {
      Uint8List encryptedOut = theCommon.theSecureMessageTransferOutgoing(
          unsecureResponse!, theAESDerivedKey!);

      logi.traceIntList("[SERVER] finally outgoing", encryptedOut);
      return encryptedOut;
    }

    logi.traceMe("[SERVER] finally outgoing, no response: return null");
    return null;
  }

  Uint8List decryptAESKey(Uint8List encryptedKey, RSAPrivateKey privateKey) {
    final decryptor = OAEPEncoding(RSAEngine())
      ..init(false, PrivateKeyParameter<RSAPrivateKey>(privateKey));
    return decryptor.process(encryptedKey);
  }

  void theKeyExchangeProcess(Socket theSock) async {
    logi.traceMe(
        'Connection from ${theSock.remoteAddress.host}:${theSock.remotePort} at handshake...');
    Map<String, dynamic>? tmpMap = get_socket_meta(theSock);
    bool needHandshake = true;
    if (tmpMap.isNotEmpty) {
      if (tmpMap.containsKey("AESkey")) {
        String tmpKey = tmpMap["AESkey"];
        if (tmpKey.isNotEmpty) {
          logi.traceMe(
              "Already have done handshake but doing anyway: // $tmpKey // ...");
        }
      }
    }

    logi.traceMe("[STCP] Sending public key? $needHandshake ..");
    if (needHandshake) {
      Uint8List publicKeyBytes = theEC.getPublicKeyAsBytes();
      logi.traceIntList("[STCP] Sending public key", publicKeyBytes);
      try {
        logi.traceIntList("Sending public key", publicKeyBytes);
        theSock.add(publicKeyBytes);
        theSock.flush();
      } catch (e) {
        logi.traceMe("Error while sending public key: $e");
      }
      logi.traceIntList("[STCP] Sent public key", publicKeyBytes);
    }
  }

  Future<void> setTheAesKey(Socket theSocket, Uint8List theAesKeyGot) async {
    Map<String, dynamic>? tmpMap = get_socket_meta(theSocket);
    if (tmpMap.isNotEmpty) {
      if (tmpMap.containsKey("AESkey")) {
        Uint8List tmpKey = tmpMap["AESkey"];
        if (tmpKey.isNotEmpty) {
          logi.traceIntList("Found AES derived key", tmpKey);
        }
      }
    }
    tmpMap["AESkey"] = theAesKeyGot;
    logi.traceIntList("Updated AES derived key", theAesKeyGot);
    set_socket_meta(theSocket, tmpMap);
  }

  Uint8List thePublicKeyGot(Socket theSocket, Uint8List thePublicKey) {
    ECPublicKey gotPubKey = theEC.bytesToPublicKey(thePublicKey);
    return theEC.computeSharedSecretAsBytes(gotPubKey);
  }

  Future<Uint8List?> theSecureMessageTransfer(
      Socket theSock, Uint8List msgIncomingCrypted) async {
    Map<String, dynamic> theMap = get_socket_meta(theSock);
    if (theMap == null) {
      logi.traceMe("Got no meta for socket! ${theSock}");
      return null;
    }
    Uint8List? theAESDerivedKey = theMap["AESkey"];

    logi.traceIntList("[SERVER] Derived AES key", theAESDerivedKey);
    logi.traceIntList("[SERVER] Data got in from AES", msgIncomingCrypted);

    Uint8List? theList = await theSecureMessageTransferAES(
        theSock, msgIncomingCrypted, theAESDerivedKey);

    logi.traceIntList("[SERVER] theSecureMessageTransferAES Returned", theList);

    if (theList == null) {
      logi.traceMe("[SERVER] Secure transfer Outgoing was null");
    }

    return theList;
  }

  StcpServer(this.host, this.port, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon();

    /*
      read() -> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) -> send()
    */
    logi.traceMe("Initialized StcpServer at $host:$port ...");
  }

  Future<void> start() async {
    await bind();
    await for (var sck in socket!) {
      _handleConnection(sck);
    }
    logi.traceMe('Server started.');
  }

  Future<void> stop() async {
    logi.traceMe('Server closed.');
  }

  Future<void> bind() async {
    socket = await ServerSocket.bind(host, port);
    logi.traceMe('Bound to ${socket!.address.address}:$port');
  }

  Map<String, Uint8List> theSharedKeys = {};
  String getKeyStringFromSock(Socket theSock) {
    return "${theSock.remoteAddress.host}:${theSock.remotePort}";
  }

  Future<void> _handleConnection(Socket theSocket) async {
    bool gotPublicKey = false;
    Uint8List? myAesKey;
    Uint8List? theSharedSecret;
    String sharedKeysKey = getKeyStringFromSock(theSocket);
    bool gotClientPublicKey = false;

    if (theSharedKeys.containsKey(sharedKeysKey)) {
      Uint8List? tmpUI = theSharedKeys[sharedKeysKey];
      if (tmpUI != null) {
        gotClientPublicKey = true;
        myAesKey = tmpUI!;
        logi.traceIntList("Got already a key for $sharedKeysKey", myAesKey);
      }
    }

//    if (!gotClientPublicKey) {
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
    logi.traceMe("[STCP] Sending my public key....");
    this.theKeyExchangeProcess(theSocket);

    subscription = theSocket.listen((theDataIn) async {
      bool okLen = theDataIn.length == 65;
      logi.traceIntList("at top of listen, data got (key? $okLen)", theDataIn);
      if ((!gotClientPublicKey) && okLen) {
        logi.traceIntList(
            "[STCP] Got public key possibly from peer: $okLen", theDataIn);
        theSharedSecret = this.thePublicKeyGot(theSocket, theDataIn);
        if (theSharedSecret != null) {
          myAesKey = theEC!.deriveSharedKeyBasedAESKey(theSharedSecret!);
          if (myAesKey != null) {
            setTheAesKey(theSocket, myAesKey!);
            logi.traceIntList("[STCP] Got public key from client", myAesKey);
            gotClientPublicKey = true;
          }
        }

        if (gotClientPublicKey == false) {
          logi.traceMe("[STCP] Re-Sending my public key....");
          this.theKeyExchangeProcess(theSocket);
        }
      } else {
        // Got public key!
        logi.traceMe("[STCP] ==== AES Traffic processing ......");
        logi.traceIntList(
            "[STCP] ==== AES theSharedSecret set", theSharedSecret);
        logi.traceIntList("[STCP] ==== AES key set", myAesKey);
        logi.traceMe("[STCP] =========================================");
        logi.traceIntList("[STCP] The AES traffic in", theDataIn);
        /*
        * Lopuksi: Clientti hanskaa AES salattuna liikenteen välittämistä
        * STCP-kerrokselle.
        */

        logi.traceMe("[STCP] Calling the message handler..");
        Uint8List? rv =
            await this.theSecureMessageTransfer(theSocket, theDataIn);
        logi.traceIntList("[STCP] Called, returned", rv);

        if (rv != null) {
          logi.traceMe("[STCP] sending returned....");
          send_raw(theSocket, rv);
        }
      }
    });
  }

  Uint8List? getAESFromSocket(Socket theSock) {
    String sharedKeysKey = this.getKeyStringFromSock(theSock);
    Uint8List? tmpUI = theSharedKeys[sharedKeysKey];

    logi.traceIntList("[$sharedKeysKey] the Shared key", tmpUI);
    if (tmpUI != null) {
      Uint8List? myAesKey = theEC!.deriveSharedKeyBasedAESKey(tmpUI!);
      logi.traceIntList("[$sharedKeysKey] theAES key", myAesKey);
      return myAesKey;
    }
    return null;
  }

  Future<Uint8List?> stcp_recv(Socket theSocket) async {
    // await initialize_socket();
    Uint8List? theAESKey = getAESFromSocket(theSocket);
    logi.traceIntList("the AES day at send", theAESKey);

    Uint8List? encryptedIn = await this.recv_raw(theSocket);

    Uint8List? theBytesOut = await theCommon.theSecureMessageTransferIncoming(
        encryptedIn!, theAESKey!);

    logi.traceIntList("STCP/recv", theBytesOut);
    return theBytesOut;
  }

  Future<void> stcp_send(Socket theSocket, Uint8List message) async {
    // await initialize_socket();
    Uint8List? theAESKey = getAESFromSocket(theSocket);
    logi.traceIntList("STCP/send/the AES day at send", theAESKey);
    logi.traceIntList("STCP/send/the message", message);

    Uint8List? theBytesOut =
        await theCommon.theSecureMessageTransferOutgoing(message, theAESKey!);
    logi.traceIntList("STCP/send/to wire:", theBytesOut);
    send_raw(theSocket, message);
  }

  Future<void> send_raw(Socket theSocket, Uint8List message) async {
    // await initialize_socket();
    logi.traceMe("Sending to $host:$port via socket $theSocket");

    String HN = theSocket.remoteAddress.host;
    int HP = theSocket.remotePort;

    logi.traceIntList("TCP/send to $HN:$HP", message);

    //Socket.connect(host, port).then((theSck) {
    theSocket.add(message);
    //  theSck.close();
    //});
  }

  Future<Uint8List?> recv_raw(Socket theSocket) async {
    // await initialize_socket();

    String HN = theSocket.address.host;
    int HP = theSocket.port;
    Uint8List? message = null;
    logi.traceMe("TCP/recv from target: $HN:$HP.....");
    RawSocket rawSocket =
        await RawSocket.connect(theSocket.address, theSocket.port);
    message = await rawSocket.read(10240);
    rawSocket.close();
    logi.traceIntList("TCP/recv ${message?.length}", message);
    return message;
  }
}
