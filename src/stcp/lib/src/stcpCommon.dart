import 'dart:typed_data';
import 'dart:math';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:the_secure_comm/the_secure_comm.dart' as scom;

/*
   IV Size: 16, 24 or 32 bytes. default: 32
*/
const int STCP_AES_KEY_SIZE_IN_BYTES = 32;
const int STCP_AES_IV_SIZE_IN_BYTES = 16;

typedef processMsgFunction  = String Function(String msgIn);

class StcpCommon {
  late String theAesKey;

  StcpCommon(String theKey) {
    theAesKey = theKey.padRight(STCP_AES_KEY_SIZE_IN_BYTES, "0").substring(0, STCP_AES_KEY_SIZE_IN_BYTES);
    logi.traceMe("AES key set: // $theAesKey //");
  }
 
  String theSecureMessageTransferIncoming(String msgIncomingCrypted) {
    String theIncomingIV = msgIncomingCrypted.substring(0, STCP_AES_IV_SIZE_IN_BYTES);
    String theEncryptedMessage = msgIncomingCrypted.substring(STCP_AES_IV_SIZE_IN_BYTES);
    scom.SecureCommunication theSCincomign = scom.SecureCommunication(theAesKey, theIncomingIV);
    String decryptedMessage = theSCincomign.decrypt(theEncryptedMessage);
    return decryptedMessage;
  }

  String theSecureMessageTransferOutgoing(String msgOutgoingPlain) {
    String theOutgoingIV = generateRandomBase64AsString(STCP_AES_IV_SIZE_IN_BYTES);
    logi.traceMe("MSG OUT KEY: ${theAesKey.length} // $theAesKey //");
    logi.traceMe("MSG OUT IV : ${theOutgoingIV.length} // $theOutgoingIV //");
    scom.SecureCommunication theSC = scom.SecureCommunication(theAesKey, theOutgoingIV);
    String encryptedOut = theOutgoingIV;
    logi.traceMe("MSG OUT A: ${msgOutgoingPlain.length} // $msgOutgoingPlain //");
    String crypted = theSC.encrypt(msgOutgoingPlain);
    logi.traceMe("MSG OUT B: ${crypted.length} // $crypted //");
    return "${encryptedOut}${crypted}";
  }

  Uint8List generateRandomBase64(int length) {
    final random = Random.secure();
    final bytes = List<int>.generate(length, (i) => random.nextInt(26) + 97);
    return Uint8List.fromList(bytes);
  }
  
  String generateRandomBase64AsString(int length) {
    final random = generateRandomBase64(length);
    return String.fromCharCodes(random);
  }
}


