import 'dart:io';
import 'dart:core';
import 'dart:async';
import 'dart:typed_data';
import 'dart:math';
import 'package:encrypt/encrypt.dart';
import 'package:pointycastle/export.dart';
import 'package:pointycastle/api.dart';
import 'package:pointycastle/pointycastle.dart';
import 'package:pointycastle/paddings/pkcs7.dart';
import 'dart:typed_data';
import 'package:crypto/crypto.dart' as crypto;
import 'package:convert/convert.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:stcp/src/stcpHeader.dart';
import 'package:the_secure_comm/the_secure_comm.dart' as scom;
import 'package:utils/utils.dart' as utils;

const bool HARD_EXEPTIONS = true;

/*
   IV Size: 16, 24 or 32 bytes. default: 32
*/
const int STCP_AES_KEY_SIZE_IN_BYTES = 32;
const int STCP_AES_IV_SIZE_IN_BYTES = 16;

const int STCP_TAG = 0xDEADBEEF;
const int STCP_HEADER_TAG_SIZE = 4;
const int STCP_HEADER_LEN_SIZE = 8;
const int STCP_HEADER_TOTAL_SIZE = STCP_HEADER_TAG_SIZE + STCP_HEADER_LEN_SIZE;
const int STCP_MAX_PACKET_SIZE_IN_BYTES = 1 << 28; // 128 MiB

const int STCP_SEARCH_HEADER_MAX_PEEK_BYTES = 32;
const int STCP_READ_TIMEOUT_IN_SECONDS = 2;
const int PAXSUDOS_SOCKET_TIMEOUT_IN_SECONDS = 5;

typedef processMsgFunction = Future<Uint8List?> Function(
    Socket theSock, Uint8List msgIn);
typedef theHandshakeFunction = void Function(Socket theSock);

typedef theHandshakeAESGotFunction = Future<void> Function(
    Socket theSocket, Uint8List theAesKeyGot);
typedef theHandshakePublicKeyGotFunction = Uint8List Function(
    Socket theSocket, Uint8List thePublicKey);

bool checkNotHeadered(Uint8List bytes, {int preview = 32}) {
  if (StcpHeader.isProbablyHeadered(bytes)) {
    logi.traceIntListIf(false, "Erroneous data head",
        bytes.sublist(0, min(preview, bytes.length)));
    throw Exception("Data is incorrectly headered at decryption step.");
  }
  return true;
}

Uint8List addHeaderToPayload(Uint8List payload) {
  if (StcpHeader.isProbablyHeadered(payload)) {
    logi.traceMeBetter("Trying to add STCP header to already headered data!");
    throw Exception("Double header detected");
  }

  final header = StcpHeader.create(payload.length);
  Uint8List out = Uint8List.fromList([...header, ...payload]);
  logi.traceIntListIf(false, "[DEBUG] Added header (TAG+LEN):", header);
  logi.traceMeIf(false, "[DEBUG] Payload length: ${payload.length}");
  logi.traceMeIf(false, "[DEBUG] Final packet length: ${out.length}");
  logi.traceIntListIf(false, "Header added in server side", out);
  return out;
}

Map<String, dynamic> removeHeaderFromPayload(Uint8List fullData) {
  if (fullData.length < StcpHeader.TOTAL_SIZE) {
    throw Exception("removeHeaderFromPayload: Data liian lyhyt");
  }

  final header = fullData.sublist(0, StcpHeader.TOTAL_SIZE);
  logi.traceIntListIf(false, "[DEBUG] Header read (TAG+LEN):", header);

  final payloadLength = StcpHeader.parseLength(header);
  logi.traceMeIf(false, "[DEBUG] Expected payload length: $payloadLength");

  final expectedLen = StcpHeader.TOTAL_SIZE + payloadLength;
  logi.traceMeIf(false, "[DEBUG] Expected total length: $expectedLen");

  if (fullData.length < expectedLen) {
    throw Exception(
        "removeHeaderFromPayload: Data does not contain STCP packet! (Expected $expectedLen bytes, got ${fullData.length} bytes)");
  }

  final payload = fullData.sublist(StcpHeader.TOTAL_SIZE, expectedLen);
  return {
    'headerWas': header,
    'payloadLength': payloadLength,
    'payload': payload,
  };
}

final Map<Socket, Stream<Uint8List>> _socketBroadcastStreams = {};

PaxsudosBufferedStreamReader CreatePaxsudosBufferedReaderFromSocket(
  Socket socket,
  Map<Socket, PaxsudosBufferedStreamReader> streamReaders,
) {
  if (streamReaders.containsKey(socket)) {
    return streamReaders[socket]!;
  }

  if (!_socketBroadcastStreams.containsKey(socket)) {
    _socketBroadcastStreams[socket] = socket.asBroadcastStream();
  }

  final stream = _socketBroadcastStreams[socket]!;

  final reader = PaxsudosBufferedStreamReader(stream);
  streamReaders[socket] = reader;
  return reader;
}

class PaxsudosBufferedStreamReader {
  List<int> _buffer = [];
  Completer<void> _ready = Completer<void>();
  late StreamSubscription<List<int>> _subscription;
  Stream<Uint8List> _input;

  /// Palauttaa `count` tavua puskuroitua dataa ilman lukuposition siirtoa.
  /// Jos dataa ei ole riittävästi, se lukee lisää `theStream`istä.
  Future<Uint8List> peekBytes(int count) async {
    if (count < 0 || count > STCP_MAX_PACKET_SIZE_IN_BYTES) {
      throw Exception("Not valid lenght: $count");
    }
    while (_buffer.length < count) {
      final chunk = await _input.first;
      if (chunk.isEmpty) {
        break;
      }
      _buffer.addAll(chunk);
    }

    if (_buffer.length < count) {
      throw Exception("peekBytes: Not enough data available");
    }

    return Uint8List.fromList(_buffer.sublist(0, count));
  }

  /// Skippaa N tavua datasta (lukee ja hylkää ne)
  Future<void> skip(int count) async {
    if (count < 0 || count > STCP_MAX_PACKET_SIZE_IN_BYTES) {
      throw Exception("Not valid lenght: $count");
    }
    int remaining = count;
    while (remaining > 0) {
      final chunk = await readBytes(remaining);
      remaining -= chunk.length;
    }
  }

  Future<bool> tryRecoverSync(
      {int maxOffset = 64,
      int maxMessageSize = STCP_MAX_PACKET_SIZE_IN_BYTES}) async {
    for (int offset = 1;
        offset <= (_buffer.length - STCP_HEADER_TOTAL_SIZE).clamp(0, maxOffset);
        offset++) {
      try {
        final candidate =
            _buffer.sublist(offset, offset + STCP_HEADER_TOTAL_SIZE);

        final tagBytes =
            Uint8List.fromList(candidate.sublist(0, STCP_HEADER_TAG_SIZE));
        final tag = ByteData.sublistView(tagBytes).getInt32(0, Endian.big);

        final lenBytes =
            Uint8List.fromList(candidate.sublist(STCP_HEADER_TAG_SIZE));

        final len = ByteData.sublistView(lenBytes).getInt64(0, Endian.big);

        if (tag == STCP_TAG &&
            len >= 0 &&
            len <= STCP_MAX_PACKET_SIZE_IN_BYTES) {
          _buffer.removeRange(0, offset);
          logi.traceMeIf(false,
              "[PaxudosSync] Found candidate header at offset $offset, len=$len");
          return true;
        }
      } catch (_) {
        // continue if any exception occurs
      }
    }

    //print("[PaxudosSync] No valid header found in first $maxOffset bytes");
    return false;
  }

  PaxsudosBufferedStreamReader(this._input) {
    if (!_input.isBroadcast) {
      throw StateError("Stream is not broadcast — use asBroadcastStream()!");
    }
    //logi.traceMeIf(false, "Adding listener....");
    try {
      _subscription = _input.listen((data) {
        //logi.traceIntListIf(false, "incoming data", data);
        _buffer.addAll(data);
        if (!_ready.isCompleted) {
          //logi.traceMeIf(false, "Settting complete");
          _ready.complete();
          //logi.traceMeIf(false, "Complete set.");
        }
      }, onDone: () {
        //logi.traceMeIf(false, "Listen onDone call...");
        if (!_ready.isCompleted) {
          _ready.complete();
        }
        //logi.traceMeIf(false, "Listen done....");
      });
    } catch (err, st) {
      logi.traceMeIf(false, "Error at listening subscription: $err");
      logi.traceTryCatch(err, theStack: st);
      if (HARD_EXEPTIONS) {
        throw Exception("Error at listening subscription: $err");
      }
    }
  }

  Future<Uint8List> readBytes(int n) async {
    //logi.traceMeIf(false, "readbytes called to read $n bytes.");
    //logi.traceMeIf(false, "BEFORE WAIT: buffer has ${_buffer.length} bytes, needs $n");

    if (n < 0 || n > STCP_MAX_PACKET_SIZE_IN_BYTES) {
      throw Exception("Not valid lenght: $n");
    }

    while (_buffer.length < n) {
      //logi.traceMeIf(false, "Receiving ${_buffer.length} / $n bytes");
      final wait = Completer<void>();
      //logi.traceMeIf(false, "Wait got..");

      _ready.future.then((_) => wait.complete());
      //logi.traceMeIf(false, "Wait setup..");

      await wait.future;
      //logi.traceMeIf(false, "Wait done..");
      _ready = Completer<void>();
      //logi.traceMeIf(false, "Got ready: ${_ready}");
    }
    List<int> tmpList = _buffer.sublist(0, n);
    final result = Uint8List.fromList(tmpList);
    //logi.traceIntListIf(false, "readBytes data got", result);
    _buffer.removeRange(0, n);

    //logi.traceMeIf(false, "AFTER READ: returning ${result.length} bytes");
    return result;
  }

  Future<Uint8List?> tryReadBytes(int len) async {
    //logi.traceMeIf(false, "readbytes called to read $len bytes.");
    //logi.traceMeIf(false,
    //    "BEFORE WAIT: buffer has ${_buffer.length} bytes, needs $len bytes");
    if (len < 0 || len > STCP_MAX_PACKET_SIZE_IN_BYTES) {
      throw Exception("Not valid lenght: $len");
    }

    while (_buffer.length < len) {
      if (_ready.isCompleted) {
        //logi.traceMeIf(false, "_ready is too early...");
        return null; // connection closed too early
      }
      await Future.delayed(Duration(milliseconds: 1));
    }

    List<int> tmpList = _buffer.sublist(0, len);
    final out = Uint8List.fromList(tmpList);
    //logi.traceIntListIf(false, "readBytes data got", out, noStr: true);
    _buffer.removeRange(0, len);
    //logi.traceMeIf(false, "AFTER READ: buffer has ${_buffer.length} bytes.");
    //logi.traceMeIf(false, "AFTER READ: returning ${out.length} bytes");
    return out;
  }

  void dispose() => _subscription.cancel();
}

Uint8List bigIntToBytes(BigInt value) {
  var hexString = value.toRadixString(16);

  if (hexString.length % 2 != 0) {
    hexString = '0$hexString';
  }

  final byteCount = hexString.length ~/ 2;
  final bytes = Uint8List(byteCount);

  for (int i = 0; i < byteCount; i++) {
    bytes[i] = int.parse(hexString.substring(i * 2, i * 2 + 2), radix: 16);
  }

  return bytes;
}

BigInt bytesToBigInt(Uint8List bytes) {
  BigInt result = BigInt.zero;
  for (int i = 0; i < bytes.length; i++) {
    result = (result << 8) | BigInt.from(bytes[i]);
  }
  return result;
}

class EllipticCodec {
  late AsymmetricKeyPair<PublicKey, PrivateKey> keys;

  EllipticCodec() {
    keys = generateKeyPair();
  }

  /// Luo AES‑avaimen (128/192/256 bit) jakosalaisuuden pohjalta.
  /// - Jos STCP_AES_KEY_SIZE_IN_BYTES == 16  → AES‑128
  /// - Jos 24                                → AES‑192
  /// - Jos 32                                → AES‑256
  Uint8List deriveSharedKeyBasedAESKey(Uint8List sharedSecret) {
    final digest = Digest('SHA-256'); // pointycastle
    final full = digest.process(sharedSecret); // 32 B = 256 bit
    assert(full.length == STCP_AES_KEY_SIZE_IN_BYTES); // 32
    return full;
  }

  Uint8List signMessage(String message, ECPrivateKey privateKey) {
    final signer = ECDSASigner(SHA256Digest());
    signer.init(true, PrivateKeyParameter(privateKey));

    final signature =
        signer.generateSignature(Uint8List.fromList(message.codeUnits))
            as ECSignature;

    final rBytes = bigIntToBytes(signature.r);
    final sBytes = bigIntToBytes(signature.s);

    return Uint8List.fromList(rBytes + sBytes);
  }

  bool verifySignature(
    String message,
    Uint8List signatureBytes,
    ECPublicKey publicKey,
  ) {
    final signer = ECDSASigner(SHA256Digest());
    signer.init(false, PublicKeyParameter(publicKey));

    // Erota r ja s allekirjoituksesta
    final r = BigInt.parse(
      hex.encode(signatureBytes.sublist(0, 32)),
      radix: 16,
    );
    final s = BigInt.parse(
      hex.encode(signatureBytes.sublist(32, 64)),
      radix: 16,
    );
    final signature = ECSignature(r, s);

    return signer.verifySignature(
      Uint8List.fromList(message.codeUnits),
      signature,
    );
  }

  Uint8List publicKeyToBytes(PublicKey thePublicKey) {
    // Hanki x- ja y-koordinaatit
    ECPublicKey publicKey = thePublicKey as ECPublicKey;
    BigInt? x = publicKey.Q?.x?.toBigInteger();
    BigInt? y = publicKey.Q?.y?.toBigInteger();

    // Muunna koordinaatit byte-taulukoksi
    final xBytes = bigIntToBytes(x!);
    final yBytes = bigIntToBytes(y!);

    // Yhdistä x- ja y-koordinaatit
    Uint8List org = Uint8List.fromList([...xBytes, ...yBytes]);

    if (org.length == 64) {
      Uint8List publicKeyBytesWithType = Uint8List(org.length + 1);
      publicKeyBytesWithType[0] = 0x04;
      publicKeyBytesWithType.setRange(1, publicKeyBytesWithType.length, org);
      org = publicKeyBytesWithType;
    }

    //utils.logAndGetStringFromBytes("publicKeyToBytes", org);
    return org;
  }

  Uint8List getPublicKeyAsBytes() {
    Uint8List org = publicKeyToBytes(keys.publicKey as ECPublicKey);
    return org;
  }

  Uint8List privateKeyToBytes(PrivateKey privateKey) {
    final d = bigIntToBytes((privateKey as ECPrivateKey).d!);
    return Uint8List.fromList(d);
  }

  // avainten luominen
  AsymmetricKeyPair<PublicKey, PrivateKey> generateKeyPair() {
    final keyGen = ECKeyGenerator();
    final params = ECKeyGeneratorParameters(ECCurve_secp256r1());
    final random = FortunaRandom();
    random.seed(KeyParameter(Uint8List(32)));
    keyGen.init(ParametersWithRandom(params, random));
    return keyGen.generateKeyPair();
  }

  ECPublicKey? bytesToPublicKey(Uint8List publicKeyBytes) {
    // Varmista, että julkinen avain alkaa oikealla tavulla

    //logi.traceIntListIf(false, "Public key raw data in", publicKeyBytes);

    if (publicKeyBytes[0] != 0x04) {
      if (HARD_EXEPTIONS) {
        throw ArgumentError("Invalid public key encoding.");
      } else {
        logi.traceMeIf(false, "Invalid public key encoding.");
        return null;
      }
    }

    // Yritetään suoraan dekoodata koko avain
    ECPoint? point = ECCurve_secp256r1().curve.decodePoint(publicKeyBytes);

    if (point == null) {
      if (HARD_EXEPTIONS) {
        throw ArgumentError("Invalid public key bytes.");
      } else {
        logi.traceMeIf(false, "Invalid public key bytes.");
        return null;
      }
    }

    // Palautetaan julkinen avain
    return ECPublicKey(point, ECCurve_secp256r1());
  }

  BigInt computeSharedSecret(PublicKey paramPublicKey) {
    final agreement = ECDHBasicAgreement();
    agreement.init(keys.privateKey as ECPrivateKey);
    return agreement.calculateAgreement(paramPublicKey as ECPublicKey);
  }

  Uint8List computeSharedSecretAsBytes(PublicKey paramPublicKey) {
    BigInt agreement = computeSharedSecret(paramPublicKey);
    Uint8List rv = bigIntToBytes(agreement);
    //logi.traceIntListIf(false, "Return value", rv);
    return rv;
  }
}

Future<Uint8List?> readCompleteStcpPacket(
    PaxsudosBufferedStreamReader reader) async {
  // Lue headeri (8 tavua)
  final headerBytes = await reader.readBytes(STCP_HEADER_TOTAL_SIZE);
  final len = StcpHeader.parseLength(headerBytes);

  logi.traceMeIf(false, "[DEBUG] AES/STCP header bytes: $headerBytes");
  logi.traceMeIf(false, "[DEBUG] AES/STCP payload length: $len");

  if (len > STCP_MAX_PACKET_SIZE_IN_BYTES || len <= 0) {
    logi.traceMeIf(false, "[DEBUG] HEADER VIRHE -> payload length = $len");
  }

  try {
    final messageLength = ByteData.sublistView(headerBytes)
        .getUint64(StcpHeader.TAG_SIZE, Endian.big);
    if (messageLength > STCP_MAX_PACKET_SIZE_IN_BYTES) {
      logi.traceMeIf(false, "[Common] Payload too large: $messageLength");
      return null;
    }

    final payloadBytes = await reader.readBytes(messageLength);
    if (payloadBytes.length != messageLength) {
      logi.traceMeIf(false,
          "[Common] Incomplete payload: got ${payloadBytes.length}/$messageLength");
      return null;
    }

    return Uint8List.fromList([...headerBytes, ...payloadBytes]);
  } catch (err, st) {
    logi.traceTryCatch(err, theStack: st);
    return null;
  }
}

class StcpCommon {
  Uint8List aesEncrypt(Uint8List key, Uint8List iv, Uint8List plain) {
    final params = PaddedBlockCipherParameters(
        ParametersWithIV<KeyParameter>(KeyParameter(key), iv), null);
    final cipher =
        PaddedBlockCipherImpl(PKCS7Padding(), CBCBlockCipher(AESEngine()))
          ..init(true, params); // encode = true
    return cipher.process(plain);
  }

  Uint8List aesDecrypt(Uint8List key, Uint8List iv, Uint8List cipherText) {
    final params = PaddedBlockCipherParameters(
        ParametersWithIV<KeyParameter>(KeyParameter(key), iv), null);
    final cipher =
        PaddedBlockCipherImpl(PKCS7Padding(), CBCBlockCipher(AESEngine()))
          ..init(false, params); // decode = false
    return cipher.process(cipherText);
  }

  /// Lähettää STCP-paketin socketille (AES tai handshake)
  Future<bool> stcp_send_packet(
    Socket sock,
    Uint8List msg, {
    Uint8List? aesKey,
    required StcpCommon theCommon,
  }) async {
    bool inAESMode = aesKey != null;
    late Uint8List wireData;

    // Älä enkryptaa 0-pituista viestiä → kirjastobugi jos teet niin
    if (msg.isEmpty) return false;

    try {
      // ======= Lähetys =======
      if (inAESMode) {
        final payload =
            theCommon.theSecureMessageTransferOutgoing(msg, aesKey)!;
        wireData = addHeaderToPayload(payload); // header AINA!
        logi.traceMeIf(false,
            "[STCP] Sending AES packet (${wireData.length} B incl header)");
      } else {
        wireData = addHeaderToPayload(msg); // handshake
      }

      logi.traceIntListIf(
          false, "[STCP] Outgoing full packet (AES Mode: $inAESMode)", wireData,
          noStr: true);

      sock.add(wireData);
      await sock.flush();
      return true;
    } catch (err, st) {
      logi.traceTryCatch(err, theStack: st);
      return false;
    }
  }

  Future<Uint8List?> stcp_recv_packet(
    PaxsudosBufferedStreamReader reader, {
    Uint8List? aesKey,
    required StcpCommon theCommon,
  }) async {
    final bool inAESMode = aesKey != null;
    // ======= Vastaanotto =======
    final full = await readCompleteStcpPacket(reader);
    if (full == null) return null;

    logi.traceIntListIf(
        false, "[STCP] Incoming full packet (AES Mode: $inAESMode)", full,
        noStr: true);

    // header -> payload
    final payload = StcpHeader.stripHeader(full);

    // Jos handshake‑paketti lipsahtaa AES‑tilaan (pituus 65 B), ohitetaan hiljaa
    if (inAESMode && payload.length == 65 && payload[0] == 0x04) {
      logi.traceMeIf(
          false, '[STCP] ⏩ Drop extra handshake pubkey (${payload.length} B)');
      return null;
    }

    if (!inAESMode) return payload; // handshake‐dataa

    // ---- AES‑dataa ----
    const ivLen = STCP_AES_IV_SIZE_IN_BYTES; // 16
    final ctLen = payload.length - ivLen;
    if (ctLen <= 0 || ctLen % 16 != 0) {
      logi.traceMeIf(
          false, '⚠︎ Drop: payload not block aligned ($ctLen B cipher)');
      return null;
    }

    return theCommon.theSecureMessageTransferIncoming(payload, aesKey!);
  }
  // Sisäinen viestien käsittely

/* ========================================================= */
/*  INCOMING  –  IV ‖ CIPHER  →  PLAIN                       */
/* ========================================================= */
  Uint8List? theSecureMessageTransferIncoming(
    Uint8List data,
    Uint8List aesKey,
  ) {
    if (data.length < STCP_AES_IV_SIZE_IN_BYTES) return null;

    final iv = data.sublist(0, STCP_AES_IV_SIZE_IN_BYTES);
    final cipher = data.sublist(STCP_AES_IV_SIZE_IN_BYTES);

    final sc = scom.SecureCommunication(aesKey, iv, 'in');
    return sc.decryptBytes(cipher); // palauttaa plain
  }

/* ========================================================= */
/*  OUTGOING  –  PLAIN  →  IV ‖ CIPHER                       */
/* ========================================================= */
  Uint8List? theSecureMessageTransferOutgoing(
    Uint8List msgOutgoingPlain,
    Uint8List aesKey,
  ) {
    // ✅ varmista pituus
    assert(aesKey.length == STCP_AES_KEY_SIZE_IN_BYTES);

    // Älä enkryptaa 0-pituista viestiä → kirjastobugi jos teet niin
    if (msgOutgoingPlain.isEmpty) return null;

    // 1. satunnainen IV
    final iv = generateRandomBytes(STCP_AES_IV_SIZE_IN_BYTES);

    // 2. salaus – käytä avainta suoraan
    final sc = scom.SecureCommunication(aesKey, iv, "Outgoing msg");
    final cipherText = sc.encryptBytes(msgOutgoingPlain);

    // 3. IV ‖ ciphertext takaisin
    return Uint8List.fromList([...iv, ...cipherText]);
  }

  Uint8List generateRandomBytes(int length) {
    if (length < 0) throw ArgumentError('length < 0');

    // Oikeasti turvallinen satunnaislähde (käyttää OS‑rng:ää)
    final rnd = Random.secure();

    // List<int>.generate luo length‑pituisen listan ja kutsuu
    //   annettua lambdaa jokaiselle indeksille.
    final bytes = List<int>.generate(length, (_) => rnd.nextInt(256));

    return Uint8List.fromList(bytes);
  }

  Uint8List generateRandomString(int length) {
    // Get random's seeded with time
    // .secure random failed so ...
    final random = Random(DateTime.timestamp().millisecondsSinceEpoch);

    final bytes = List<int>.generate(length, (i) => random.nextInt(25) + 65);
    return Uint8List.fromList(bytes);
  }
}
