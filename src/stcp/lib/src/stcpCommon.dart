import 'dart:io';
import 'dart:typed_data';
import 'dart:math';
import 'package:encrypt/encrypt.dart';
import 'package:pointycastle/export.dart';
import 'package:pointycastle/api.dart';
import 'package:pointycastle/pointycastle.dart';
import 'dart:typed_data';
import 'package:crypto/crypto.dart' as crypto;
import 'package:convert/convert.dart';
import 'dart:convert';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_secure_comm/the_secure_comm.dart' as scom;

/*
   IV Size: 16, 24 or 32 bytes. default: 32
*/
const int STCP_AES_KEY_SIZE_IN_BYTES = 32;
const int STCP_AES_IV_SIZE_IN_BYTES = 16;

typedef processMsgFunction =
    Uint8List? Function(Socket theSock, Uint8List msgIn);
typedef theHandshakeFunction = void Function(Socket theSock);

typedef theHandshakeAESGotFunction =
    Future<void> Function(Socket theSocket, Uint8List theAesKeyGot);
typedef theHandshakePublicKeyGotFunction =
    Uint8List Function(Socket theSocket, Uint8List thePublicKey);

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

  String deriveSharedKeyBasedAESKey(Uint8List paramSharedSecret) {
    // Käytä HMAC-avain
    var hmac = crypto.Hmac(crypto.sha256, paramSharedSecret);

    crypto.Digest aesKeyList = hmac.convert(
      Uint8List(STCP_AES_KEY_SIZE_IN_BYTES),
    );
    String out = String.fromCharCodes(aesKeyList.bytes);
    return base64Encode(out.codeUnits).substring(0, STCP_AES_KEY_SIZE_IN_BYTES);
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

    logi.logAndGetStringFromBytes("publicKeyToBytes", org);
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

  ECPublicKey bytesToPublicKey(Uint8List publicKeyBytes) {
    // Varmista, että julkinen avain alkaa oikealla tavulla
    if (publicKeyBytes[0] != 0x04) {
      throw ArgumentError("Invalid public key encoding.");
    }

    // Yritetään suoraan dekoodata koko avain
    ECPoint? point = ECCurve_secp256r1().curve.decodePoint(publicKeyBytes);
    if (point == null) {
      throw ArgumentError("Invalid public key bytes.");
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
    return bigIntToBytes(agreement);
  }
}

class StcpCommon {
  Uint8List theSecureMessageTransferIncoming(
    Uint8List msgIncomingCrypted,
    String theAESpresharedkey,
  ) {
    logi.logAndGetStringFromBytes(
      "theSecureMessageTransferIncoming raw",
      msgIncomingCrypted,
    );

    String theAesKey = theAESpresharedkey
        .padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0")
        .substring(0, STCP_AES_KEY_SIZE_IN_BYTES);

    logi.traceMe("The AES key in incoming: $theAesKey");
    String strIncomingPacket = logi.logAndGetStringFromBytes(
      "the IV vector + Payload of message in",
      msgIncomingCrypted,
    );

    Uint8List theIncomingIV = logi.logAndGetBytesFromString(
      "the incoming IV-vector",
      strIncomingPacket.substring(0, STCP_AES_IV_SIZE_IN_BYTES),
    );

    Uint8List theEncryptedMessage = logi.logAndGetBytesFromString(
      "the Encrypted message",
      strIncomingPacket.substring(STCP_AES_IV_SIZE_IN_BYTES),
    );

    scom.SecureCommunication theSCincomign = scom.SecureCommunication(
      theAesKey,
      theIncomingIV,
      name: "Incoming AES traffic",
    );

    logi.logAndGetBytesFromString("theAesKey", theAesKey);
    logi.logAndGetStringFromBytes("theIncomingIV", theIncomingIV.toList());
    logi.logAndGetStringFromBytes(
      "encrypted msg",
      theEncryptedMessage.toList(),
    );
    String decryptedMessage = theSCincomign.decrypt(theEncryptedMessage);
    Uint8List tmp = logi.logAndGetBytesFromString(
      "Decrypted message",
      decryptedMessage,
    );
    return tmp;
  }

  Uint8List theSecureMessageTransferOutgoing(
    Uint8List msgOutgoingPlain,
    String theAESpresharedkey,
  ) {
    String theAesKey = theAESpresharedkey
        .padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0")
        .substring(0, STCP_AES_KEY_SIZE_IN_BYTES);

    Uint8List theOutgoingIV = generateRandomBase64(STCP_AES_IV_SIZE_IN_BYTES);

    logi.traceMe("MSG OUT KEY: ${theAesKey.length} // $theAesKey //");
    logi.traceMe("MSG OUT IV : ${theOutgoingIV.length} // $theOutgoingIV //");
    logi.logAndGetStringFromBytes("MSG OUT raw plain", msgOutgoingPlain);

    scom.SecureCommunication theSC = scom.SecureCommunication(
      theAesKey,
      theOutgoingIV,
      name: "Outgoing msg",
    );

    String outgoingIVvector = logi.logAndGetStringFromBytes(
      "MSG OUT raw IV Vector",
      theOutgoingIV.toList(),
    );

    String encryptedOut = logi.logAndGetStringFromBytes(
      "MSG OUT raw out going payload",
      msgOutgoingPlain.toList(),
    );

    String cryptedMessage = theSC.encrypt(msgOutgoingPlain);

    logi.logAndGetBytesFromString("MSG OUT raw crypted", cryptedMessage);

    String outMessage = outgoingIVvector + cryptedMessage;
    Uint8List tmp = logi.logAndGetBytesFromString(
      "MSG out [Outgoing IV] + [crypted message]",
      outMessage,
    );
    return tmp;
  }

  Uint8List generateRandomBase64(int length) {
    // Get random's seeded with time
    // .secure random failed so ...
    final random = Random(DateTime.timestamp().millisecondsSinceEpoch);

    final bytes = List<int>.generate(length, (i) => random.nextInt(26) + 97);
    return Uint8List.fromList(bytes);
  }

  String generateRandomBase64AsString(int length) {
    final random = generateRandomBase64(length);
    return String.fromCharCodes(random);
  }
}
