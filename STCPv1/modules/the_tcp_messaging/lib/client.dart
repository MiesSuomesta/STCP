import 'dart:io';
import 'dart:convert';
import 'dart:async';
import 'dart:typed_data';
import 'package:encrypt/encrypt.dart' as enc;
import 'package:pointycastle/export.dart';
import 'package:pointycastle/pointycastle.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_secure_comm/the_secure_comm.dart' as scom;

import 'package:stcp/stcp.dart';

class TcpClient {
  final String host;
  final int port;
  late EllipticCodec theEC;
  theHandshakeFunction theHandshakeHandler;
  processMsgFunction theMessageHandler;
  theHandshakeAESGotFunction theAesGotCallback;
  theHandshakePublicKeyGotFunction thePublicKeyGotCallback;
  Socket? socket;

  TcpClient(
      this.host,
      this.port,
      this.theEC,
      this.theMessageHandler,
      this.theHandshakeHandler,
      this.theAesGotCallback,
      this.thePublicKeyGotCallback) {
    logi.traceMe("Setting up client for $host:$port");
    logi.traceMe(
        "MSCB: $theMessageHandler HSH: $theHandshakeHandler, AG: $theAesGotCallback");
    logi.traceMe("PKGOT: $thePublicKeyGotCallback");
  }

  Future<void> connect() async {
//    try {

    bool needsServerPublicKey = true;
    logi.traceMe('connecting to server: $host:$port ....');
    socket = await Socket.connect(host, port);
    logi.traceMe(
        'connected to server: ${socket!.address.host}:${socket!.port} .. socket: $socket');

    logi.traceMe("[Socket listen (Client)] Starting listening ....");
    socket?.listen((theData) {
      logi.logAndGetStringFromBytes("incoming data", theData);
      logi.traceMe("Got incoming data ${theData.length} bytes");

      if (needsServerPublicKey) {
        if (theData.length != 65) {
          logi.traceMe("[STCP] =========================================");
          logi.traceMe("[STCP] ==== Handshake process, no valid data...");
          logi.traceMe("[STCP] =========================================");
          return;
        }

        logi.traceMe(
            "[Socket listen (Client)] Sending my public key to server..");
        Uint8List thePubKeyBytes = theEC.getPublicKeyAsBytes();
        logi.logAndGetStringFromBytes(
            "The public key of client", thePubKeyBytes);

        // Serveri on lähettänyt public keyn
        ECPublicKey theServerKey = theEC.bytesToPublicKey(theData);
        Uint8List theSharedKey = theEC.computeSharedSecretAsBytes(theServerKey);

        logi.logAndGetStringFromBytes("Shared key", theSharedKey);

        logi.traceMe("[Socket listen (Client)] Computed shared secret ....");
        String theNewAesKey = theEC.deriveSharedKeyBasedAESKey(theSharedKey);
        Uint8List myKeyList = logi.logAndGetBytesFromString(
            "New AES key from shared data", theNewAesKey);

        logi.traceMe("[STCP] Got sharedkey, setting it to be AES key,"
            " passing it to STCP layer .....");

        theAesGotCallback(socket!, myKeyList);
        logi.traceMe("[Socket listen (Client)] Set AES key ....");

        /* lähetetään clientin public key */

        needsServerPublicKey = false;

        send(thePubKeyBytes);

        return;
      }

      logi.traceMe("[Socket listen (Client)] Passing to STCP handler");
      Uint8List? retFromHandler = this.theMessageHandler(socket!, theData);
      if (retFromHandler != null) {
        String out = logi.logAndGetStringFromBytes(
            "Send after handler call", retFromHandler);
        logi.traceMe("Sending now data from message handler...");
        send(retFromHandler);
      } else {
        logi.traceMe("Got nothing to send from message handler....");
      }
    });
  }

  send(Uint8List message) {
    logi.traceMe("Sending to $host:$port via socket $socket");
    int len = 512;
    if (len > message.length) {
      len = message.length;
    }
    String HN = host;
    int HP = port;

    logi.logAndGetStringFromBytes("Sending to wire for $HN:$HP", message);
    Socket.connect(host, port).then((theSck) {
      theSck.add(message);
      theSck.close();
    });
  }

  Future<void> disconnect() async {
    if (socket != null) {
      await socket?.close();
    }
    logi.traceMe('Disconnected.');
  }
}
