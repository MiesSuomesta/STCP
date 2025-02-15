import 'dart:convert';
import 'dart:typed_data';
import 'package:utils/utils.dart' as util;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_simple_http/the_simple_http.dart' as SH;

import './thePaxsudosMessage.dart' as pm;
import './sharedSettings.dart' as sset;

import 'dart:io';

typedef processMsgFunction = dynamic Function(dynamic msgIn);

class TcpClient {
  final String host;
  final int port;
  final processMsgFunction theMessageHandler;
  Socket? _socket;

  TcpClient(this.host, this.port, this.theMessageHandler);

  Future<void> connect() async {

    _socket = await Socket.connect(
      host,
      port,
    );
    print('connected to server: ${_socket!.remoteAddress.address}:$port');

    _socket!.listen(
      (data) {
        theMessageHandler(msg);
      },
      onDone: () {
        print('Connection closed..');
      },
    );
  }

  void send(String message) {
    _socket?.write(message);
  }

  Future<void> disconnect() async {
    await _socket?.close();
    print('Disconnected.');
  }
}

dynamic clientProcessMessage(dynamic msgIn) {
  return rv;
}

void main() async {
  while (true) {
    try {
      var client = TcpClient('localhost', 15000, clientProcessMessage);
      await client.connect();

      pm.PaxsudosMessage msgOut = pm.PaxsudosMessage();
      msgOut.fromSlices("service", "Luokka", "teretere!");

      // Lähetä viesti palvelimelle
      client.send(msgOut.msgData);

      // Sulje yhteys viiden sekunnin kuluttua
      await Future.delayed(Duration(seconds: 5));
      await client.disconnect();
    } catch (err) {
      logi.traceMe("Client error: $err");
    }
  }
}
