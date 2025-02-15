import 'dart:io';
import 'dart:convert';
import 'dart:typed_data';
import 'package:utils/utils.dart' as util;
import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_simple_http/the_simple_http.dart' as SH;

import './thePaxsudosMessage.dart' as pm;
import './sharedSettings.dart' as sset;

import 'dart:io';

typedef processMsgFunction = dynamic Function(pm.PaxsudosMessage msgIn);

class TcpServer {
  final int port;
  final processMsgFunction theMessageHandler;
  SecureServerSocket? _serverSocket;

  TcpServer(this.theMessageHandler, this.port);

  Future<void> start() async {
    var context = SecurityContext();
    context.useCertificateChain('certificate.pem');
    context.usePrivateKey('private_key.pem');

    _serverSocket = await SecureServerSocket.bind(
      InternetAddress.anyIPv4,
      port,
      context,
    );
    print('Palvelin käynnistetty: ${_serverSocket!.address.address}:$port');

    await for (var socket in _serverSocket!) {
      _handleConnection(socket);
    }
  }

  void _handleConnection(Socket socket) {
    print(
        'Yhteys vastaanotettu: ${socket.remoteAddress.address}:${socket.remotePort}');
    socket.listen(
      (data) {
        String? rv = null;
        print('Vastaanotettu: ${String.fromCharCodes(data)}');
        if (pm.PaxsudosMessage.is_raw_a_message(data)) {
          pm.PaxsudosMessage msg = pm.PaxsudosMessage();
          msg.fillFromRaw(data);
          rv = theMessageHandler(msg);
        }
        if (rv != null) {
          logi.traceMe("Got RV: $rv");
          socket.write(rv);
        }
      },
      onDone: () {
        print(
            'Yhteys suljettu: ${socket.remoteAddress.address}:${socket.remotePort}');
      },
    );
  }

  Future<void> stop() async {
    await _serverSocket?.close();
    print('Palvelin suljettu.');
  }
}

String? processMessageCallback(pm.PaxsudosMessage theMsg) {
  // List<int> rv = theMsg.asUint8List();
  logi.traceMe("Got message: // ${theMsg.msgData} //");
  return theMsg.msgData;
}

void main() async {
  var server = TcpServer(processMessageCallback, 15000);
  await server.start();
}
