// Linux glibc STCP-TCP echo client. Dart SDK only.
import 'dart:convert';
import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';
import 'echo_server.dart' as stcp;

final connect = stcp.libc.lookupFunction<
    Int32 Function(Int32, Pointer<Void>, Uint32),
    int Function(int, Pointer<Void>, int)>('connect');

void run(List<String> args) {
  if (args.isEmpty || args.length > 3) {
    throw const FormatException('usage: echo_client.dart IPv4 [port] [message]');
  }
  final ip = InternetAddress.tryParse(args[0]);
  if (ip == null || ip.type != InternetAddressType.IPv4) {
    throw const FormatException('IPv4 literal required');
  }
  final port = args.length > 1 ? int.parse(args[1]) : 7000;
  if (port < 1 || port > 65535) throw const FormatException('port must be 1..65535');
  final payload = utf8.encode(args.length > 2 ? args[2] : 'Hello STCP!');
  if (payload.isEmpty) throw const FormatException('message must not be empty');
  final client = stcp.check(stcp.socket(45, 1, 253), 'socket');
  Pointer<Void> address = nullptr;
  Pointer<Void> storage = nullptr;
  try {
    address = stcp.calloc(1, 16);
    storage = stcp.calloc(1, 4096);
    if (address == nullptr || storage == nullptr) throw const OSError('allocation failed');
    final bytes = address.cast<Uint8>().asTypedList(16);
    final data = ByteData.sublistView(bytes);
    data.setUint16(0, 2, Endian.host);
    data.setUint16(2, port, Endian.big);
    bytes.setRange(4, 8, ip.rawAddress);
    stcp.check(connect(client, address, 16), 'connect');
    final buffer = storage.cast<Uint8>();
    for (var start = 0; start < payload.length; start += 4096) {
      final end = start + 4096 < payload.length ? start + 4096 : payload.length;
      final length = end - start;
      buffer.asTypedList(length).setRange(0, length, payload, start);
      var offset = 0;
      while (offset < length) {
        final count = stcp.send(client, (buffer + offset).cast(), length - offset, 0x4000);
        if (count < 0 && stcp.errnoLocation().value == 4) continue;
        stcp.check(count, 'send');
        if (count == 0) throw const OSError('zero-byte send');
        offset += count;
      }
      offset = 0;
      while (offset < length) {
        final count = stcp.recv(client, (buffer + offset).cast(), length - offset, 0);
        if (count < 0 && stcp.errnoLocation().value == 4) continue;
        stcp.check(count, 'recv');
        if (count == 0) throw const OSError('connection closed before complete echo');
        offset += count;
      }
      final reply = buffer.asTypedList(length);
      for (var i = 0; i < length; i++) {
        if (reply[i] != payload[start + i]) throw const FormatException('echo mismatch');
      }
    }
    stdout.writeln('PASS: ${payload.length} bytes echoed unchanged');
  } finally {
    stcp.free(storage);
    stcp.free(address);
    stcp.close(client);
  }
}
void main(List<String> args) {
  try { run(args); } catch (error) { stderr.writeln(error); exitCode = 1; }
}
