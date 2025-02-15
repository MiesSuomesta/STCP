import 'dart:convert';
import 'dart:typed_data';
import 'dart:math';
import 'dart:typed_data';

import 'package:encrypt/encrypt.dart';

import 'package:utils/utils.dart' as util;
import 'package:paxlog/paxlog.dart' as logi;


const int SECURE_COMM_IV_LENGHT_IN_BYTES = 16;
const int SECURE_COMM_KEY_LENGHT_IN_BYTES = 32;
class SecureCommunication {
  final Key key;
  final IV iv;

  SecureCommunication(String keyString, String ivString)
      : key = Key.fromUtf8(keyString),
        iv = IV.fromUtf8(ivString);

  String encrypt(String plainText) {
    final encrypter = Encrypter(AES(key));
    final encrypted = encrypter.encrypt(plainText, iv: iv);
    return encrypted.base64;
  }

  String decrypt(String encryptedText) {
    final encrypter = Encrypter(AES(key));
    final decrypted = encrypter.decrypt64(encryptedText, iv: iv);
    return decrypted;
  }
}
