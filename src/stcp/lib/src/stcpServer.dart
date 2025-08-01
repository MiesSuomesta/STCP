library stcp;

import 'dart:convert';
import 'dart:io';
import 'dart:async';
import 'dart:math';
import 'dart:typed_data';
import 'package:stcp/src/stcpHeader.dart';
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

  void forceCloseSocket(Socket socket) {
    try {
      socket.destroy();
      logi.traceMeIf(false, "Socket destroyed.");
    } catch (err, st) {
      logi.traceMeIf(false, "Error while destoroying socket: $err");
      logi.traceTryCatch(err, theStack: st);
    }
    logi.traceMeIf(false, "Dispose reader & controller...");
    streamReaders.remove(socket)?.dispose();
    socketControllers.remove(socket)?.close();
    logi.traceMeIf(false, "Socket removed from maps.");
  }

  Future<void> restartHandshake(Socket theSock) async {
    logi.traceMeIf(false, "[STCP] Restarting handshake....");
    logi.traceMeIf(false, "Removing Shared key ..");
    try {
      theSharedKeys.remove(theSock);
      logi.traceMeIf(false, "Force socket close.");
    } catch (err, st) {
      logi.traceMeIf(false, "Error while removing info of socket: $err");
      logi.traceTryCatch(err, theStack: st);
    }
    forceCloseSocket(theSock);
  }

  Future<Uint8List?> theSecureMessageTransferAES(Socket theSock,
      Uint8List msgIncomingCrypted, Uint8List? theAESDerivedKey) async {
    logi.traceIntListIf(false, "Secure message transfer AES input", msgIncomingCrypted);
    //logi.traceIntListIf(false, "The AES key", theAESDerivedKey);
    if (theAESDerivedKey == null) {
      throw Exception(
          "STCP Protocol init failure: AES secret is not set to metadata!");
    }

    late Uint8List? decryptedMessage;
    try {
      decryptedMessage = theCommon.theSecureMessageTransferIncoming(
          msgIncomingCrypted, theAESDerivedKey!);
      if (decryptedMessage == null) {
        if (HARD_EXEPTIONS) {
          throw Exception("Errror while processing incoming message!");
        } else {
          logi.traceMeIf(false, "Errror while processing incoming message!");
          return null;
        }
      }
    } catch (e, st) {
      logi.traceMeIf(false, "[STCP] AES decrypt failed: $e\n$st");
      logi.traceTryCatch(e, theStack: st);
      // Kutsu handshake-prosessia uudelleen
      await restartHandshake(theSock);
      return null;
    }

    logi.traceIntListIf(false, "[SERVER] AES message decrypted", decryptedMessage);

    // the unsecure call
    logi.traceMeIf(false, "Undesure callback call ===================================");
    logi.traceIntListIf(false, "[SERVER] UNSEC to process", decryptedMessage);

    Uint8List? unsecureResponse =
        await theUnsecureMessageHandler(theSock, decryptedMessage);

    logi.traceIntListIf(false, "[SERVER] unsecure callback returned", unsecureResponse);

    if (unsecureResponse != null) {
      if (StcpHeader.isProbablyHeadered(unsecureResponse)) {
        logi.traceMeIf(false, 
            "[SERVER] Response already headered. Removing header before encrypt.");
        final map = removeHeaderFromPayload(unsecureResponse);
        unsecureResponse = map["payload"];
      }
    }

    //logi.traceMeIf(false, "Undesure callback done ===================================");

    if (unsecureResponse != null) {
      logi.traceIntListIf(false, "[SERVER] going to out transfer", unsecureResponse);

      Uint8List? encryptedOut = theCommon.theSecureMessageTransferOutgoing(
          unsecureResponse!, theAESDerivedKey!);
      if (encryptedOut == null) {
        if (HARD_EXEPTIONS) {
          await restartHandshake(theSock);
          throw Exception("Errror while processing outgoing message!");
        } else {
          logi.traceMeIf(false, "Errror while processing outgoing message!");
        }
        await restartHandshake(theSock);
        return null;
      }

      logi.traceIntListIf(false, "[SERVER] outgoing encrypted message", encryptedOut);
      return encryptedOut;
    }

    //logi.traceMeIf(false, "[SERVER] finally outgoing, no response: return null");
    return null;
  }

  Uint8List decryptAESKey(Uint8List encryptedKey, RSAPrivateKey privateKey) {
    final decryptor = OAEPEncoding(RSAEngine())
      ..init(false, PrivateKeyParameter<RSAPrivateKey>(privateKey));
    return decryptor.process(encryptedKey);
  }

  Map<Socket, bool> thePublicKeySentForSocketDone = {};
  Uint8List? thePublicKeyGot(Socket theSocket, Uint8List thePublicKey) {
    ECPublicKey? gotPubKey = theEC.bytesToPublicKey(thePublicKey);
    if (gotPubKey == null) {
      if (HARD_EXEPTIONS) {
        throw Exception("Error while validating public key...");
      } else {
        logi.traceMeIf(false, "Error while validating public key...");
        return null;
      }
    }
    return theEC.computeSharedSecretAsBytes(gotPubKey);
  }

  Future<Uint8List?> theSecureMessageTransfer(
      Socket theSock, Uint8List msgIncomingCrypted) async {
    Uint8List? theAESDerivedKey = getAESFromSocket(theSock);

    //logi.traceIntListIf(false, "[SERVER] Derived AES key", theAESDerivedKey);
    //logi.traceIntListIf(false, "[SERVER] Data got in from AES", msgIncomingCrypted);

    Uint8List? theList = await theSecureMessageTransferAES(
        theSock, msgIncomingCrypted, theAESDerivedKey);

    logi.traceIntListIf(false, "[SERVER] theSecureMessageTransferAES Returned", theList);

    if (theList == null) {
      //logi.traceMeIf(false, "[SERVER] Secure transfer Outgoing was null");
    }

    return theList;
  }

  StcpServer(this.host, this.port, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon();

    /*
      read() -> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) -> send()
    */
    logi.traceMeIf(false, "Initialized StcpServer at $host:$port ...");
  }

  Future<void> start() async {
    await bind();
    await for (var sck in socket!) {
      sck.timeout(
        Duration(seconds: PAXSUDOS_SOCKET_TIMEOUT_IN_SECONDS),
        onTimeout: (sink) {
          logi.traceMeIf(false, 
              "Nothing heard in $PAXSUDOS_SOCKET_TIMEOUT_IN_SECONDS minutes, closing...");
          sck.destroy(); // Katkaise yhteys, jos ei kuulu mitään minuuttiin
        },
      );

      String hostt = sck.remoteAddress.address;
      int port = sck.port;

      logi.traceMeIf(false, 
          "New socket from $host:$port with timeout of $PAXSUDOS_SOCKET_TIMEOUT_IN_SECONDS minutes");

      _handleConnection(sck);
    }
    //logi.traceMeIf(false, 'Server started.');
  }

  Future<void> stop() async {
    //logi.traceMeIf(false, 'Server closed.');
  }

  Future<void> bind() async {
    socket = await ServerSocket.bind(host, port);
    //logi.traceMeIf(false, 'Bound to ${socket!.address.address}:$port');
  }

  Map<Socket, Uint8List> theSharedKeys = {};

  void setAESKeyOfSocket(Socket theSock, Uint8List theAESKey) {
    assert(theAESKey.length == STCP_AES_KEY_SIZE_IN_BYTES);
    theSharedKeys[theSock] = theAESKey;
  }

  void removeAESKeyOfSocket(Socket theSock) {
    theSharedKeys.remove(theSock);
  }

  Uint8List? getAESKeyOfSocket(Socket theSock) {
    return theSharedKeys[theSock];
  }

  Future<void> theKeyExchangeProcess(Socket sock, StcpCommon common) async {
    // 0. lähetä oma julkinen avain vain kerran per soketti
    if (!thePublicKeySentForSocketDone.containsKey(sock)) {
      final pubBytes = theEC.getPublicKeyAsBytes();
      await common.stcp_send_packet(sock, pubBytes,
          aesKey: null, theCommon: common);
      thePublicKeySentForSocketDone[sock] = true;
      logi.traceMeIf(false, '[STCP] ⇢  sent server pubkey (65 B)');
    }

    final reader = CreatePaxsudosBufferedReaderFromSocket(sock, streamReaders);

    /* ------- 1. odota CLIENT‑PUBKEY -------- */
    Uint8List clientPubKey;
    while (true) {
      final payload = await common.stcp_recv_packet(reader,
          aesKey: null, theCommon: common);
      if (payload == null) continue;

      if (payload.length == 65 && payload[0] == 0x04) {
        clientPubKey = payload;
        logi.traceMeIf(false, '[STCP] ⇠  client pubkey received');
        break;
      }
    }

    // 2. laske shared secret  →  derive AES‑key
    final ecPub = theEC.bytesToPublicKey(clientPubKey)!;
    final secret = theEC.computeSharedSecretAsBytes(ecPub);
    final aesKey = theEC.deriveSharedKeyBasedAESKey(secret);

    setAESKeyOfSocket(sock, aesKey); // talteen karttaan

    /* ------- 3. lähetä AES("READY") -------- */
    await common.stcp_send_packet(sock, Uint8List.fromList('READY'.codeUnits),
        aesKey: aesKey, theCommon: common);

    /* ------------------------------------------------------------------
     *  ODOTA CLIENTILTÄ KUITTAUS
     * ------------------------------------------------------------------ */
    bool readyAcked = false;
    Uint8List? firstAppMsg;
    while (!readyAcked) {
      final plain = await common.stcp_recv_packet(reader,
          aesKey: aesKey, theCommon: common);
      if (plain == null) continue;

      if (utf8.decode(plain) == 'READY') {
        readyAcked = true;
        logi.traceMeIf(false, '[STCP] ⇠  client READY OK → handshake done (Server)');
      } else {
        // saatiin oikea data ennen READY-kuittausta
        firstAppMsg = plain;
        readyAcked = true;
        logi.traceMeIf(false, '[STCP] ⇠  client sent data before READY – hyväksytään');
      }
    }

    /* Handshake valmis – siirry viestin­käsittelyyn */
    if (firstAppMsg != null) {
      final reply = await theUnsecureMessageHandler(sock, firstAppMsg);
      if (reply != null) {
        await theCommon.stcp_send_packet(sock, reply,
            aesKey: getAESKeyOfSocket(sock), theCommon: theCommon);
      }
    }

    // nyt voit siirtyä varsinaiseen viestienkäsittelyyn
  }

  Future<void> _handleConnection(Socket socket) async {
    final sockID = "${socket.remoteAddress.address}:${socket.remotePort}";
    logi.traceMeIf(false, "[STCP/$sockID] New connection");

    /* ------------------------------------------------------------------
   * 0.  Buffered reader per socket
   * ------------------------------------------------------------------ */
    final reader = streamReaders.putIfAbsent(
        socket,
        () => CreatePaxsudosBufferedReaderFromSocket(
              socket,
              streamReaders,
            ));

    await theKeyExchangeProcess(socket, theCommon);

    Uint8List? theAESKey = getAESKeyOfSocket(socket);

    if (theAESKey == null) {
      logi.traceMeIf(false, "[STCP/$sockID] No AES key!");
      return;
    }

    /* ------------------------------------------------------------------
     *  AES‑tilan pääsilmukka
     * ------------------------------------------------------------------ */
    try {
      // Odota JOKO aesLoopin päättymistä TAI socket.done‑Futuren täyttymistä
      await Future.any(
          [handleAESMessagesInLoop(socket, reader, theAESKey), socket.done]);
    } catch (e, st) {
      logi.traceMeIf(false, "[STCP/$sockID] connection error → ${e}");
      logi.traceTryCatch(e, theStack: st);
    } finally {
      // odota että OS sulkee yhteyden (EOF) ja siivoa resurssit
      await socket.close();
      reader.dispose();
      streamReaders.remove(socket);
      removeAESKeyOfSocket(socket);
      logi.traceMeIf(false, "[STCP/$sockID] Connection closed");
    }
  }

  /* Puhtaampi AES‑silmukka */
  Future<void> handleAESMessagesInLoop(Socket socket,
      PaxsudosBufferedStreamReader reader, Uint8List aesKey) async {
    bool sockClosed = false;
    final sockID = "${socket.remoteAddress.address}:${socket.remotePort}";

    socket.done.then((_) {
      sockClosed = true;
      logi.traceMeIf(false, "[STCP/$sockID] Closing AES mode socket!");
    });

    try {
      while (!sockClosed) {
        final plain = await theCommon.stcp_recv_packet(reader,
            aesKey: getAESKeyOfSocket(socket), theCommon: theCommon);

        if (plain == null) {
          if (sockClosed) break; // EOF
          continue; // ei vielä kokonaista pakettia
        }

        final reply = await theUnsecureMessageHandler(socket, plain);
        if (reply != null) {
          await theCommon.stcp_send_packet(socket, reply,
              aesKey: getAESKeyOfSocket(socket), theCommon: theCommon);
        }
      }
    } catch (e, st) {
      logi.traceTryCatch(e, theStack: st);
    }
  }

  Uint8List? getAESFromSocket(Socket theSock) {
    return theSharedKeys[theSock];
  }

  // Ei nätti mut joooh....
  final Map<Socket, PaxsudosBufferedStreamReader> streamReaders = {};
  final Map<Socket, StreamController<Uint8List>> socketControllers = {};

  void disposeSocketReader(Socket socket) {
    logi.traceMeIf(false, "Removing socket from readers..");
    streamReaders.remove(socket)?.dispose();
    socketControllers.remove(socket);
  }

  Future<Uint8List?> recv_raw(Socket socket,
      {int? theLength,
      int timeoutInSeconds = STCP_READ_TIMEOUT_IN_SECONDS}) async {
    // Alustetaan buffered reader jos puuttuu
    streamReaders[socket] ??=
        CreatePaxsudosBufferedReaderFromSocket(socket, streamReaders);

    // Poista reader kun socket sulkeutuu
    socket.done.then((_) {
      forceCloseSocket(socket);
    });

    final reader = streamReaders[socket]!;

    try {
      if (theLength != null) {
        logi.traceMeIf(false, "Getting raw $theLength bytes");
        final ret = await reader
            .tryReadBytes(theLength)
            .timeout(Duration(seconds: timeoutInSeconds));
        if (ret == null || ret.isEmpty) return null;

        logi.traceIntListIf(false, 
            "recv_raw() received buffer", ret.sublist(0, min(ret.length, 64)));
        logi.traceIntListIf(false, "RAW data read", ret, noStr: true);
        return ret;
      }

      logi.traceMeIf(false, "Reading complete STCP packet...");

      Uint8List? received = await readCompleteStcpPacket(reader);

      if (received == null || received.isEmpty) {
        logi.traceMeIf(false, "Received null (likely disconnect).");
        return null;
      }

      final header = received.sublist(0, STCP_HEADER_TOTAL_SIZE);
      logi.traceIntListIf(false, "Received header from client", header);
      logi.traceIntListIf(false, "recv_raw() received buffer",
          received.sublist(0, min(received.length, 64)));
      return received;
    } catch (err) {
      logi.traceMeIf(false, "recv_raw error: $err");
      return null;
    }
  }
}
