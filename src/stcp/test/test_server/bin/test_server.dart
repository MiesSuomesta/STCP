import 'dart:io';
import 'dart:convert';
import 'dart:typed_data';
import 'package:stcp/stcp.dart' as stcp;

Future<void> main(List<String> args) async {
  final host = args.isNotEmpty ? args[0] : 'localhost';
  final port = args.length > 1 ? int.parse(args[1]) : 8888;

  print("DEBUG STCP server starting at $host:$port...");
  final server = stcp.StcpServer(host, port, handleMessage);
  await server.start();
}

Future<Uint8List?> handleMessage(Socket sock, Uint8List data) async {
  final msg = utf8.decode(data, allowMalformed: true);
  print("[SERVER] Got message (${data.length} bytes): $msg");

  // Lähetä kuittaus takaisin
  final reply = "Server ACK: $msg";
  return Uint8List.fromList(utf8.encode(reply));
}
