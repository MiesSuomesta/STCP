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
import 'package:utils/utils.dart' as utils;

/*
   IV Size: 16, 24 or 32 bytes. default: 32
*/
const int STCP_AES_KEY_SIZE_IN_BYTES = 32;
const int STCP_AES_IV_SIZE_IN_BYTES = 16;

typedef processMsgFunction = Future<Uint8List?> Function(
    Socket theSock, Uint8List msgIn);
typedef theHandshakeFunction = void Function(Socket theSock);

typedef theHandshakeAESGotFunction = Future<void> Function(
    Socket theSocket, Uint8List theAesKeyGot);
typedef theHandshakePublicKeyGotFunction = Uint8List Function(
    Socket theSocket, Uint8List thePublicKey);

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

  Uint8List deriveSharedKeyBasedAESKey(Uint8List paramSharedSecret) {
    // Käytä HMAC-avain
    var hmac = crypto.Hmac(crypto.sha256, paramSharedSecret);

    crypto.Digest aesKeyList = hmac.convert(
      Uint8List(STCP_AES_KEY_SIZE_IN_BYTES),
    );
    String out = String.fromCharCodes(aesKeyList.bytes);
    String theRV =
        base64Encode(out.codeUnits).substring(0, STCP_AES_KEY_SIZE_IN_BYTES);
    Uint8List rv = Uint8List.fromList(theRV.codeUnits);
    return rv;
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

  ECPublicKey bytesToPublicKey(Uint8List publicKeyBytes) {
    // Varmista, että julkinen avain alkaa oikealla tavulla

    //logi.traceIntList("Public key raw data in", publicKeyBytes);

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
    Uint8List rv = bigIntToBytes(agreement);
    //logi.traceIntList("Return value", rv);
    return rv;
  }
}

class StcpCommon {
  Uint8List theSecureMessageTransferIncoming(
    Uint8List msgIncomingCrypted,
    Uint8List theAESpresharedkeyList,
  ) {
    logi.traceIntList("MSG TRANSFER incoming", theAESpresharedkeyList);

    // Erotetaan IV ja salattu viesti binäärisesti
    final theIncomingIV =
        msgIncomingCrypted.sublist(0, STCP_AES_IV_SIZE_IN_BYTES);
    final theEncryptedMessage =
        msgIncomingCrypted.sublist(STCP_AES_IV_SIZE_IN_BYTES);

    logi.traceIntList("MSG TRANSFER incoming IV", theIncomingIV);
    logi.traceIntList("MSG TRANSFER incoming enc MSG", theEncryptedMessage);

    // Luodaan SecureCommunication olio käyttäen binäärisiä IV:tä ja avainta
    final sc = scom.SecureCommunication(
      theAESpresharedkeyList,
      theIncomingIV,
      "Incoming AES traffic",
    );

    // Puretaan salattu viesti binäärinä
    final decryptedBytes = sc.decryptBytes(theEncryptedMessage);

    logi.traceIntList("MSG TRANSFER incoming decrypt", decryptedBytes,
        nLenMax: 1024 * 2000);

    return decryptedBytes;
  }

  Uint8List theSecureMessageTransferOutgoing(
    Uint8List msgOutgoingPlain,
    Uint8List theAESpresharedkey,
  ) {
    // Pad key to exact length
    String keyStr = String.fromCharCodes(theAESpresharedkey)
        .padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0")
        .substring(0, STCP_AES_KEY_SIZE_IN_BYTES);
    Uint8List theAesKey = Uint8List.fromList(keyStr.codeUnits);

    // Luo satunnainen IV (täsmälleen oikean mittainen binäärinä)
    Uint8List theOutgoingIV = generateRandomBytes(STCP_AES_IV_SIZE_IN_BYTES);

    logi.traceIntList("MSG OUT KEY", theAesKey);
    logi.traceIntList("MSG OUT IV", theOutgoingIV);
    logi.traceIntList("MSG OUT raw plain", msgOutgoingPlain);

    // Luo AES-salaaja
    final theSC = scom.SecureCommunication(
      theAesKey,
      theOutgoingIV,
      "Outgoing msg",
    );

    // Salaa binääridata
    final encryptedMessage = theSC.encryptBytes(msgOutgoingPlain);

    // Liitä IV + salattu viesti peräkkäin
    final outMessage =
        Uint8List.fromList([...theOutgoingIV, ...encryptedMessage]);

    logi.traceIntList("MSG out [IV + crypted]", outMessage);

    return outMessage;
  }

  Uint8List generateRandomBytes(int length) {
    // Get random's seeded with time
    // .secure random failed so ...
    final random = Random(DateTime.timestamp().millisecondsSinceEpoch);

    final bytes = List<int>.generate(length, (i) => random.nextInt(255));
    return Uint8List.fromList(bytes);
  }
}
