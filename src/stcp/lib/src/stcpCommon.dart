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

    utils.logAndGetStringFromBytes("publicKeyToBytes", org);
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

    logi.traceIntList("Public key raw data in", publicKeyBytes);

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
    logi.traceIntList("Return value", rv);
    return rv;
  }
}

class StcpCommon {
  Uint8List theSecureMessageTransferIncoming(
    Uint8List msgIncomingCrypted,
    Uint8List theAESpresharedkeyList,
  ) {
    logi.traceIntList("MSG TRANSFER incoming", theAESpresharedkeyList);
    String theAESpresharedkey =
        String.fromCharCodes(theAESpresharedkeyList.toList());

    String theAesKey = theAESpresharedkey
        .padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0")
        .substring(0, STCP_AES_KEY_SIZE_IN_BYTES);

    logi.traceMe("The AES key in incoming: $theAesKey");
    String strIncomingPacket = utils.logAndGetStringFromBytes(
      "the IV vector + Payload of message in",
      msgIncomingCrypted,
    );

    Uint8List theIncomingIV = utils.logAndGetBytesFromString(
      "the incoming IV-vector",
      strIncomingPacket.substring(0, STCP_AES_IV_SIZE_IN_BYTES),
    );
    logi.traceIntList("MSG TRANSFER incoming IV", theIncomingIV);

    Uint8List theEncryptedMessage = utils.logAndGetBytesFromString(
      "the Encrypted message",
      strIncomingPacket.substring(STCP_AES_IV_SIZE_IN_BYTES),
    );

    logi.traceIntList("MSG TRANSFER incoming enc MSG", theEncryptedMessage);

    String theIV = String.fromCharCodes(theIncomingIV.toList());

    scom.SecureCommunication theSCincomign = scom.SecureCommunication(
      theAesKey,
      theIV,
      "Incoming AES traffic",
    );

    utils.logAndGetBytesFromString("theAesKey", theAesKey);
    utils.logAndGetStringFromBytes("theIncomingIV", theIncomingIV);
    logi.traceIntList("the Encrypted message", theEncryptedMessage);

    String theEncryptedMessageStr =
        String.fromCharCodes(theEncryptedMessage.toList());
    String decryptedMessage = theSCincomign.decrypt(theEncryptedMessageStr);

    logi.traceIntList("MSG TRANSFER incoming decrypt",
        Uint8List.fromList(decryptedMessage.codeUnits));

    Uint8List tmp = utils.logAndGetBytesFromString(
      "Decrypted message",
      decryptedMessage,
    );
    return tmp;
  }

  Uint8List theSecureMessageTransferOutgoing(
    Uint8List msgOutgoingPlain,
    Uint8List theAESpresharedkey,
  ) {
    logi.traceIntList("MSG TRANSFER incoming", theAESpresharedkey);
    String theAESpresharedkeyStr =
        String.fromCharCodes(theAESpresharedkey.toList());

    String theAesKey = theAESpresharedkeyStr
        .padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0")
        .substring(0, STCP_AES_KEY_SIZE_IN_BYTES);
    Uint8List theAesKeyUintList = Uint8List.fromList(theAesKey.codeUnits);

    logi.traceMe("The AES key in incoming: $theAesKey");

    Uint8List theOutgoingIV = generateRandomBase64(STCP_AES_IV_SIZE_IN_BYTES);

    logi.traceMeBetter("MSG OUT KEY: ${theAesKey.length} // $theAesKey //");
    logi.traceMe("MSG OUT IV : ${theOutgoingIV.length} // $theOutgoingIV //");
    utils.logAndGetStringFromBytes("MSG OUT raw plain", msgOutgoingPlain);

    String theOutgoingIVStr = String.fromCharCodes(theOutgoingIV.toList());

    scom.SecureCommunication theSC = scom.SecureCommunication(
      theAesKey,
      theOutgoingIVStr,
      "Outgoing msg",
    );

    String outgoingIVvector = utils.logAndGetStringFromBytes(
      "MSG OUT raw IV Vector",
      theOutgoingIV,
    );

    String encryptedOut = utils.logAndGetStringFromBytes(
      "MSG OUT raw out going payload",
      msgOutgoingPlain,
    );

    String cryptedMessage = theSC.encrypt(encryptedOut);

    utils.logAndGetBytesFromString("MSG OUT raw crypted", cryptedMessage);

    String outMessage = outgoingIVvector + cryptedMessage;
    Uint8List tmp = utils.logAndGetBytesFromString(
      "MSG out [Outgoing IV] + [crypted message]",
      outMessage,
    );

    logi.traceMe("==============================================");
    logi.traceMe("== lja1 ");
    logi.traceMe("==============================================");
    logi.traceMe("MSG decrypt try ============================");
    this.theSecureMessageTransferIncoming(tmp, theAesKeyUintList);
    logi.traceMe("MSG decrypt try end ========================");

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
