import 'dart:typed_data';
import 'package:encrypt/encrypt.dart';
import 'package:paxlog/paxlog.dart' as logi;

const int SECURE_COMM_IV_LENGHT_IN_BYTES = 16;
const int SECURE_COMM_KEY_LENGHT_IN_BYTES = 32;

bool DEBUG_LOG_PLAIN = false;
bool DEBUG_LOG_CRYPTED = false;

class SecureCommunication {
  // ------------------------------------------------------------
  // Pituus­varmistukset:  32 B (256 bit) avain  &  16 B IV
  // ------------------------------------------------------------
  static Uint8List _normalizeKey(Uint8List src) {
    if (src.length == SECURE_COMM_KEY_LENGHT_IN_BYTES) return src;
    if (src.length > SECURE_COMM_KEY_LENGHT_IN_BYTES) {
      // liian pitkä  → katkaise
      return Uint8List.sublistView(src, 0, SECURE_COMM_KEY_LENGHT_IN_BYTES);
    }
    // liian lyhyt  → täydennä nollilla (tai muulla kaavalla)
    return Uint8List.fromList(
      src + List.filled(SECURE_COMM_KEY_LENGHT_IN_BYTES - src.length, 0),
    );
  }

  static Uint8List _normalizeIV(Uint8List src) {
    if (src.length == SECURE_COMM_IV_LENGHT_IN_BYTES) return src;
    if (src.length > SECURE_COMM_IV_LENGHT_IN_BYTES) {
      return Uint8List.sublistView(src, 0, SECURE_COMM_IV_LENGHT_IN_BYTES);
    }
    return Uint8List.fromList(
      src + List.filled(SECURE_COMM_IV_LENGHT_IN_BYTES - src.length, 0),
    );
  }

  final Key key;
  final IV iv;
  final String sName;

  SecureCommunication(Uint8List keyBytes, Uint8List ivBytes, this.sName)
      : key = Key(_normalizeKey(keyBytes)),
        iv = IV(_normalizeIV(ivBytes));

  // --- salaus ---
  Uint8List encryptBytes(Uint8List plainBytes) {
    if (DEBUG_LOG_PLAIN) {
      logi.traceIntList("[$sName] plain ⇒ encrypt", plainBytes);
    }
    final enc = Encrypter(AES(key, mode: AESMode.cbc)); // PKCS7‑padding
    final ct = enc.encryptBytes(plainBytes, iv: iv);

    if (DEBUG_LOG_CRYPTED) {
      logi.traceIntList("[$sName] cipher", ct.bytes);
    }
    return ct.bytes; // IV liitetään protokollatasolla
  }

  // --- purku ---
  Uint8List decryptBytes(Uint8List cipherBytes) {
    if (DEBUG_LOG_CRYPTED) {
      logi.traceIntList("[$sName] cipher ⇒ decrypt", cipherBytes);
    }
    final enc = Encrypter(AES(key, mode: AESMode.cbc));
    final pt = enc.decryptBytes(Encrypted(cipherBytes), iv: iv);
    final out = Uint8List.fromList(pt);

    if (DEBUG_LOG_PLAIN) {
      logi.traceIntList("[$sName] plain", out);
    }
    return out;
  }
}
