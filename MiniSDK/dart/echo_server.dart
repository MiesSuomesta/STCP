// Linux glibc STCP-TCP echo server. Dart SDK only (no pub packages).
import 'dart:ffi';
import 'dart:io';
import 'dart:typed_data';

final libc = DynamicLibrary.open('libc.so.6');
final socket = libc.lookupFunction<Int32 Function(Int32, Int32, Int32),
    int Function(int, int, int)>('socket');
final bind = libc.lookupFunction<Int32 Function(Int32, Pointer<Void>, Uint32),
    int Function(int, Pointer<Void>, int)>('bind');
final listen = libc.lookupFunction<Int32 Function(Int32, Int32),
    int Function(int, int)>('listen');
final accept = libc.lookupFunction<Int32 Function(Int32, Pointer<Void>, Pointer<Void>),
    int Function(int, Pointer<Void>, Pointer<Void>)>('accept');
final recv = libc.lookupFunction<IntPtr Function(Int32, Pointer<Void>, UintPtr, Int32),
    int Function(int, Pointer<Void>, int, int)>('recv');
final send = libc.lookupFunction<IntPtr Function(Int32, Pointer<Void>, UintPtr, Int32),
    int Function(int, Pointer<Void>, int, int)>('send');
final close = libc.lookupFunction<Int32 Function(Int32), int Function(int)>('close');
final calloc = libc.lookupFunction<Pointer<Void> Function(UintPtr, UintPtr),
    Pointer<Void> Function(int, int)>('calloc');
final free = libc.lookupFunction<Void Function(Pointer<Void>),
    void Function(Pointer<Void>)>('free');
final errnoLocation = libc.lookupFunction<Pointer<Int32> Function(),
    Pointer<Int32> Function()>('__errno_location');

int check(int result, String operation) {
  if (result < 0) throw OSError(operation, errnoLocation().value);
  return result;
}
void echo(int client, Pointer<Uint8> buffer) {
  while (true) {
    final count = recv(client, buffer.cast(), 65536, 0);
    if (count < 0 && errnoLocation().value == 4) continue; // EINTR
    check(count, 'recv');
    if (count == 0) return;
    var offset = 0;
    while (offset < count) {
      final sent = send(client, (buffer + offset).cast(), count - offset, 0x4000);
      if (sent < 0 && errnoLocation().value == 4) continue;
      check(sent, 'send'); // MSG_NOSIGNAL = 0x4000 on Linux
      if (sent == 0) throw const OSError('zero-byte send');
      offset += sent;
    }
  }
}
void run(List<String> args) {
  if (args.length > 1) throw const FormatException('usage: echo_server.dart [port]');
  final port = args.isEmpty ? 7000 : int.parse(args.single);
  if (port < 1 || port > 65535) throw const FormatException('port must be 1..65535');
  final server = check(socket(45, 1, 253), 'socket');
  Pointer<Void> address = nullptr;
  Pointer<Void> storage = nullptr;
  try {
    address = calloc(1, 16);
    storage = calloc(1, 65536);
    if (address == nullptr || storage == nullptr) throw const OSError('allocation failed');
    final bytes = address.cast<Uint8>().asTypedList(16);
    final data = ByteData.sublistView(bytes);
    data.setUint16(0, 2, Endian.host); // sockaddr_in: AF_INET
    data.setUint16(2, port, Endian.big);
    check(bind(server, address, 16), 'bind');
    check(listen(server, 16), 'listen');
    stdout.writeln('STCP-TCP echo listening on 0.0.0.0:$port');
    while (true) {
      final client = accept(server, nullptr, nullptr);
      if (client < 0 && [4, 32, 103, 104, 107, 110, 111].contains(errnoLocation().value)) continue;
      check(client, 'accept');
      try {
        echo(client, storage.cast<Uint8>());
      } on OSError catch (error) {
        stderr.writeln('client: $error');
      } finally {
        close(client);
      }
    }
  } finally {
    free(storage);
    free(address);
    close(server);
  }
}
void main(List<String> args) {
  try {
    run(args);
  } catch (error) {
    stderr.writeln(error);
    exitCode = 1;
  }
}
