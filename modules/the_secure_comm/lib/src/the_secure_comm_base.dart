import 'dart:convert';
import 'dart:typed_data';
import 'dart:math';
import 'dart:typed_data';

import 'package:encrypt/encrypt.dart';
import 'package:crypto/crypto.dart';
import 'package:paxlog/paxlog.dart' as logi;
import 'package:pointycastle/padded_block_cipher/padded_block_cipher_impl.dart';

import 'package:stcp/stcp.dart';

class SecureCommunication {
  final Key key;
  final IV iv;

  SecureCommunication(String keyString, Uint8List ivBytes,
      {String name = "SCOM"})
      : key = Key.fromUtf8(keyString.substring(0, STCP_AES_KEY_SIZE_IN_BYTES)),
        iv = IV.fromUtf8(String.fromCharCodes(ivBytes.toList())
            .substring(0, STCP_AES_IV_SIZE_IN_BYTES)) {
    logi.traceMe(
        "[$name] Setting SecComm: $keyString // AES KEY LEN: ${keyString.length} // IV LEN: ${ivBytes.length} // ");
  }

  String encrypt(Uint8List plainData) {
    logi.traceMe("Encrypting from: ${plainData.length} // $plainData");
    String incomingPlain = logi.logAndGetStringFromBytes(
        "Encrypting incoming", plainData.toList());

    logi.logAndGetStringFromBytes(
        "GOT KEY: ${key.bytes.length}", key.bytes.toList());
    logi.logAndGetStringFromBytes(
        "GOT IV: ${iv.bytes.length}", iv.bytes.toList());

    final encrypter = Encrypter(AES(key, padding: null));
    final encrypted = encrypter.encrypt(incomingPlain, iv: iv);

    logi.logAndGetStringFromBytes("ecrypted data", encrypted.bytes.toList());

    String theBE = base64Encode(encrypted.bytes);

    String outBytes = logi.logAndGetStringFromBytes(
        "BASE64 ecrypted data", theBE.codeUnits.toList());

    return outBytes;
  }

  String decrypt(Uint8List encryptedData) {
    String inBytes = logi.logAndGetStringFromBytes(
        "BASE64 ecrypted data", encryptedData.toList());

    Uint8List tmpBasedDecoded = base64Decode(inBytes);

    String encPayloadBE =
        logi.logAndGetStringFromBytes("BASE64 decoded data", tmpBasedDecoded);

    final decrypter = Encrypter(AES(key, padding: null));
    Encrypted theEncrypted = Encrypted(tmpBasedDecoded);
    final decrypted = decrypter.decrypt(theEncrypted, iv: iv);
    logi.logAndGetStringFromBytes(
        "Decrypted string", decrypted.codeUnits.toList());
    return decrypted;
  }
}
