import 'dart:io';
import 'dart:convert';
import 'dart:typed_data';
import 'package:paxlog/paxlog.dart' as logi;
import 'package:stcp/stcp.dart' as stcp;

Future<void> main(List<String> args) async {
  final host = args.isNotEmpty ? args[0] : 'localhost';
  final port = args.length > 1 ? int.parse(args[1]) : 8888;

  print("DEBUG STCP client connecting to $host:$port...");
  final client = stcp.StcpClient(host, port, (sock, data) async {
    print(
      "[CLIENT-ASYNC] Received async (${data.length} bytes): ${utf8.decode(data)}",
    );
    return null;
  });

  await client.connect();
  print('[CLIENT] Connected...');

  await client.waitUntilConnectionReady();
  print('[CLIENT] Connected – AES mode enabled.');

  final msg = "Hello debug AES!";
  print("[CLIENT] Sending message: $msg");
  await client.stcp_send(Uint8List.fromList(utf8.encode(msg)));

  print('[CLIENT] Kirjoita viesti (tyhjä rivi = exit)');

  client.theSocket.done.then((_) {
    logi.traceMe("Socket closed.");
    client.stop();
  });

  while (true) {
    final line = stdin.readLineSync(encoding: utf8) ?? '';
    if (line.isEmpty) break; // tyhjä rivi => lopetus

    await client.stcp_send(Uint8List.fromList(line.codeUnits));

    // odotetaan vastaus
    final Uint8List? ans = await client.stcp_recv();
    if (ans == null) {
      print('[CLIENT] No reply (connection closed)');
      break;
    }
    print('[CLIENT] ⇐ ${utf8.decode(ans)}');
  }

  await client.stop();
  print('[CLIENT] Closed.');
}
