library stcp;

import 'dart:convert';
import 'dart:io';
import 'dart:core';
import 'dart:async';
import 'dart:typed_data';
import 'package:pointycastle/export.dart';
import 'package:pointycastle/pointycastle.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:stcp/src/stcpHeader.dart';
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
  late final StreamController<Uint8List> _controller;
  late final Stream<Uint8List> theInputStream;
  late final PaxsudosBufferedStreamReader theInputStreamReader;

  final Completer<void> _ready = Completer<void>();

  bool isReconnecting = false;
  bool connectionFailed = false;

  Future<bool> reconnect({int retryDelayMs = 1000, int maxRetries = 5}) async {
    if (isReconnecting || theSocketIsInitted) return false;

    isReconnecting = true;
    logi.traceMeIf(false, "🔄 Trying to reconnect to $host:$port...");

    int retries = 0;
    while (retries < maxRetries) {
      try {
        await initialize_socket();
        logi.traceMeIf(false, "Reconnected OK");
        isReconnecting = false;
        connectionFailed = false;
        return true;
      } catch (e) {
        logi.traceMeIf(false, "Reconnect failed ($retries): $e");
        await Future.delayed(Duration(milliseconds: retryDelayMs));
        retries++;
      }
    }

    logi.traceMeIf(false, "Reconnect failed.");
    isReconnecting = false;
    connectionFailed = true;
    return false;
  }

  Future<void> theKeyExchangeProcess(
      Socket theSock, StcpCommon theCommon) async {
    logi.traceMeIf(false,
        'Connection from ${theSock.address.host}:${theSock.port} at handshake...');

    // Oma public key
    Uint8List publicKeyBytes = theEC.getPublicKeyAsBytes();

    // Lähetetään public key - nyt käytetään stcp_send_packet() ilman AES-keytä
    bool ok = await theCommon.stcp_send_packet(
      theSock,
      publicKeyBytes,
      aesKey: null, // Handshake-tilassa ei AES
      theCommon: theCommon,
    );

    if (!ok) {
      logi.traceMeIf(false, "KeyExchange send failed!");
      theSock.destroy();
      return;
    }

    logi.traceMeIf(false, "[STCP] Sent public key (handshake) successfully.");
  }

  Future<void> setTheAesKey(Socket theSocket, Uint8List theAesKeyGot) async {
    //logi.traceMeIf(false, "Setting the AES key...............");
    theAesKey = theAesKeyGot;
    //logi.traceIntListIf(false, "the AES key set to", theAesKey);
  }

  Uint8List? thePublicKeyGot(Socket theSocket, Uint8List thePublicKey) {
    if (theEC != null) {
      ECPublicKey? gotPubKey = theEC!.bytesToPublicKey(thePublicKey);

      if (gotPubKey == null) {
        if (HARD_EXEPTIONS) {
          throw Exception("Not a public key!");
        } else {
          logi.traceMeIf(false, "Not a public key!");
          return null;
        }
      }

      return theEC.computeSharedSecretAsBytes(gotPubKey);
    } else {
      return null;
    }
  }

  Future<Uint8List?> theSecureMessageTransfer(
      Socket theSock, Uint8List msgIncomingCrypted) async {
    //logi.traceIntListIf(false, "MSG transfer/plain", msgIncomingCrypted);
    //String theAESpresharedkey = theAesKey;
    Uint8List? decryptedMessage;

    try {
      // Erottele STCP-header, IV ja salattu viesti
      final header = StcpHeader.fromBytes(
          msgIncomingCrypted.sublist(0, STCP_HEADER_TOTAL_SIZE));
      if (header == null) {
        logi.traceMeIf(false, "Invalid STCP header in incoming data.");
        return null;
      }

      final int payloadLength = header.length;
      final Uint8List encryptedPayload =
          msgIncomingCrypted.sublist(STCP_HEADER_TOTAL_SIZE);

      if (encryptedPayload.length != payloadLength) {
        logi.traceMeIf(false,
            "Payload size mismatch: expected $payloadLength, got ${encryptedPayload.length}");
        return null;
      }

      // Decryptaa vain IV + salattu viesti
      decryptedMessage = theCommon.theSecureMessageTransferIncoming(
          encryptedPayload, theAesKey!);
    } catch (err, st) {
      logi.traceMeIf(false, "Error while processing incoming data?");
      logi.traceTryCatch(err, theStack: st);
    }

    if (decryptedMessage == null) {
      if (HARD_EXEPTIONS) {
        throw Exception("Errror while processing incoming message!");
      } else {
        logi.traceMeIf(false, "Errror while processing incoming message!");
        return null;
      }
    }

    String decryptedMessageDecodedString = utf8.decode(decryptedMessage);
    Uint8List decryptedMessageDecoded =
        Uint8List.fromList(decryptedMessageDecodedString.codeUnits);

    logi.traceIntListIf(
        false, "MSG transfer/decrypted to process", decryptedMessage);

    // the unsecure call
    Uint8List? unsecureResponse;
    try {
      unsecureResponse =
          await theUnsecureMessageHandler(theSock, decryptedMessageDecoded);
      if (unsecureResponse != null) {
        if (StcpHeader.isProbablyHeadered(unsecureResponse)) {
          logi.traceIntListIf(
              false, "[STCP/Client] HEADER in RESPONSE", unsecureResponse,
              noStr: true);
          logi.traceMeBetter("Stack trace of re-headered...");
        }
      }
    } catch (err, st) {
      logi.traceMeIf(false, "Error while processing in unsecure callback.");
      logi.traceTryCatch(err, theStack: st);
      return null;
    }

    if (unsecureResponse != null) {
      logi.traceIntListIf(
          false, "MSG transfer/decrypted processed", unsecureResponse!);
      if (theAesKey != null) {
        Uint8List? encryptedOut = theCommon.theSecureMessageTransferOutgoing(
            unsecureResponse!, theAesKey!);

        if (encryptedOut == null) {
          if (HARD_EXEPTIONS) {
            throw Exception("Errror while processing outgoing message!");
          } else {
            logi.traceMeIf(false, "Errror while processing outgoing message!");
            return null;
          }
        }

        logi.traceIntListIf(false, "MSG transfer/crypted out", encryptedOut);
        return encryptedOut;
      }
    }
    return null;
  }

  bool theSocketIsInitted = false;
  Future<bool> initialize_socket() async {
    if (theSocketIsInitted) return false;
    theSocketIsInitted = true;

    theSocket = await Socket.connect(host, port,
        timeout: Duration(seconds: PAXSUDOS_SOCKET_TIMEOUT_IN_SECONDS));

    logi.traceMeIf(false,
        "Made new socket, with timeout of $PAXSUDOS_SOCKET_TIMEOUT_IN_SECONDS minutes");

    // Alusta controller ja broadcast-stream
    _controller = StreamController<Uint8List>.broadcast();
    theInputStream = _controller.stream;

    theInputStreamReader = PaxsudosBufferedStreamReader(theInputStream);

    // Aloita kuuntelu
    theSocket.listen(
      (data) {
        _controller.add(data);
      },
      onDone: () async {
        logi.traceMeIf(false, "Socket done: Stream broke. Reconnecting...");
        _controller.close();
        theSocketIsInitted = false;
        await reconnect();
      },
      onError: (e) async {
        logi.traceMeIf(false, "Socket stream error: $e, reconnecting....");
        _controller.addError(e);
        theSocketIsInitted = false;
        await reconnect();
      },
      cancelOnError: true,
    );

    return true;
  }

  StcpClient(this.host, this.port, this.theUnsecureMessageHandler) {
    theCommon = StcpCommon();
    theAesKey = Uint8List.fromList([0]);

    //logi.traceMeIf(false, "Init of StcpClient: $host:$port");

    /*
      | <--- done at recv ---> |                | <--- done at send ---> |
      read() --> SECURE(decrypt) -> INSECURE -> SECURE(encrypt) --> send()
    */

    //logi.traceMeIf(false, "Initialized StcpClient");
  }

  /// Luo TCP‑yhteyden, suorittaa STCP‑handshaken ja
  /// asettaa `theAesKey` valmiiksi AES‑tilaa varten.
  /// Heittää poikkeuksen, jos jokin vaihe epäonnistuu.
  Future<void> connect() async {
    await initialize_socket(); // luo theSocket, theInputStreamReader

    if (theEC == null) {
      throw StateError("STCP init failure: Elliptic curve helper missing");
    }

    /* ------------------------------------------------------------------
     * 1) vastaanota serverin julkinen avain
     * ------------------------------------------------------------------ */
    Uint8List? srvPub;
    final handshakeTimeout = DateTime.now().add(const Duration(seconds: 5));

    while (srvPub == null) {
      // Aikakatkaisu
      if (DateTime.now().isAfter(handshakeTimeout)) {
        throw TimeoutException("Handshake timed out (server pubkey)");
      }

      srvPub = await theCommon.stcp_recv_packet(
        theInputStreamReader, // PaxsudosBufferedStreamReader
        aesKey: null, // = handshake
        theCommon: theCommon,
      );
    }

    if (srvPub.length != 65 || srvPub[0] != 0x04) {
      throw FormatException("Handshake: invalid pubkey (${srvPub.length} B)");
    }
    logi.traceIntListIf(false, "[STCP] ⇐ server pubkey", srvPub, noStr: true);

    /* ------------------------------------------------------------------
   * 2) lähetä OMA julkinen avain plain-tilassa
   * ------------------------------------------------------------------ */
    await theCommon.stcp_send_packet(
      theSocket,
      theEC.getPublicKeyAsBytes(), // 65 B, alkaa 0x04
      aesKey: null, // -> plain header
      theCommon: theCommon,
    );
    logi.traceMeIf(false, '[STCP] ⇢  client pubkey sent');

    /* ------------------------------------------------------------------
   * 3) odota SERVERIN salattu READY
   * ------------------------------------------------------------------ */
    Uint8List? ivPlusCipher;
    while (true) {
      ivPlusCipher = await theCommon.stcp_recv_packet(
        theInputStreamReader,
        aesKey: null, // vielä plain-tila
        theCommon: theCommon,
      );

      if (ivPlusCipher != null) {
        if (ivPlusCipher.length >= 32) break; // tod.näk. IV|CT
      }
    }

    /* ------------------------------------------------------------------
     * 4) johda ja aseta AES-avain vasta nyt
     * ------------------------------------------------------------------ */
    final sharedSecret = thePublicKeyGot(theSocket, srvPub)!;
    theAesKey = theEC.deriveSharedKeyBasedAESKey(sharedSecret);
    await setTheAesKey(theSocket, theAesKey!);
    logi.traceIntListIf(false, '[STCP] ✔︎ AES key', theAesKey!, noStr: true);

    /* ------------------------------------------------------------------
     * 5) pura saatu READY-paketti ja varmista että se on “READY”
     * ------------------------------------------------------------------ */
    final plainReady = theCommon.theSecureMessageTransferIncoming(
      ivPlusCipher,
      theAesKey!,
    );
    if (utf8.decode(plainReady!) != 'READY') {
      throw StateError('Handshake: server did not send READY');
    }
    logi.traceMeIf(false, '[STCP] ⇐ server READY OK');

    /* ------------------------------------------------------------------
     * 6) vastaa omalla AES-salatulla READY-paketilla
     * ------------------------------------------------------------------ */
    await theCommon.stcp_send_packet(
      theSocket,
      Uint8List.fromList('READY'.codeUnits),
      aesKey: theAesKey!,
      theCommon: theCommon,
    );

    logi.traceMeIf(false, '[STCP] ⇢  client READY sent – handshake DONE');

    _ready.complete();
    logi.traceMeIf(false, '[STCP] ⇢  client send/recv enabled!');
  }

  Future<void> waitUntilConnectionReady() => _ready.future;

  Future<bool> stcp_send(Uint8List message) async {
    await initialize_socket();

    if (!_ready.isCompleted) {
      throw StateError("Handshake is not done yet!");
    }

    try {
      await theCommon.stcp_send_packet(
        theSocket,
        message,
        aesKey: theAesKey, // voi olla null
        theCommon: theCommon,
      );
    } catch (e, st) {
      logi.traceTryCatch(e, theStack: st);
      return false;
    }
    return true;
  }

  Future<Uint8List?> stcp_recv() async {
    await initialize_socket();

    if (!_ready.isCompleted) {
      throw StateError("Handshake is not done yet!");
    }

    final reader = theInputStreamReader;
    return await theCommon.stcp_recv_packet(
      reader,
      aesKey: theAesKey, // voi olla null
      theCommon: theCommon,
    );
  }

  Future<void> stop() async {
    logi.traceMeIf(false, "Socket close....");
    await theSocket.close();
    //theSocket.destroy();
    //logi.traceMeIf(false, "Socket reset....");
    this.theSocketIsInitted = false;
  }

  Future<void> breakConnections() async {
    //logi.traceMeIf(false, "Socket destroy....");
    logi.traceMeIf(false, "Socket destroy!");
    theSocket.destroy();
    //logi.traceMeIf(false, "Socket reset....");
    this.theSocketIsInitted = false;
  }

  void setMessageHandler(
      Future<Uint8List?> Function(Socket theSock, Uint8List msgIn)
          thePlaintextCommandResponseHandler) {
    this.theUnsecureMessageHandler = thePlaintextCommandResponseHandler;
  }
}
