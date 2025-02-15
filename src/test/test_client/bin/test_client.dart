import 'dart:io' as os;

import 'package:args/args.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:stcp/stcp.dart' as stcp;
import 'package:the_secure_comm/the_secure_comm.dart' as sc;

void main(List<String> arguments) async {
  // Luo ArgParser-olio
  final parser = ArgParser();

  String serverHostname = "localhost";
  int serverPort = 8888;
  int sendingIntervalMS = 1500;
  String txt = "Testing string of data 1234567890" ;
  String aes = "Insecure? But secure :D";
  bool doLooping = false;
  bool doHelp = false;
  bool doTestSC = false;
  stcp.StcpCommon theStcpCommon = stcp.StcpCommon("nullkey");


  // Lisää argumentteja parseriin
  parser.addOption('host', help: 'server hostname');
  parser.addOption('port', help: 'server port');
  parser.addOption('txt', help: 'Text to send');
  parser.addOption('interval', help: 'interval to send in ms');
  parser.addOption('aes', help: 'AES encryption key, must be same at server instance');
  parser.addFlag  ('loop', help: 'Enable looping');
  parser.addFlag  ('test-sc', help: 'Test SecureCommunication.');
  parser.addFlag  ('help', help: 'Usage');

  // Parsitaan argumentit
  try {
    
    ArgResults argResults = parser.parse(arguments);
    serverHostname = argResults['host'] ?? "localhost";
    serverPort = int.parse(argResults['port'] ?? "8888");
    txt = argResults['txt'] ?? "Testing string of data that has length of some packets 11223344556677889900 1234567890" ;

    String keyString = theStcpCommon.theAesKey; // 32 tavua
    aes = argResults['aes'] ?? keyString;
    doLooping = argResults['loop'];
    doHelp = argResults['help'];
    doTestSC = argResults['test-sc'];

  } catch (err) {
    print('Usage:');
    print(parser.usage);
    os.exit(1);
  }

  logi.traceMe("Using AES key: $aes");

  if (doHelp) {
    print('Usage:');
    print(parser.usage);
    os.exit(0);
  }

  if (doTestSC) {
    // Esimerkki avaimen ja IV:n luomisesta
    String keyString = theStcpCommon.generateRandomBase64AsString(stcp.STCP_AES_KEY_SIZE_IN_BYTES); // 32 tavua
    String ivString = theStcpCommon.generateRandomBase64AsString(stcp.STCP_AES_IV_SIZE_IN_BYTES); // 16 tavua

    sc.SecureCommunication secureComm = sc.SecureCommunication(keyString, ivString);
    
    String encrypted = secureComm.encrypt("Hello, World! 13452370478183947510938745012394572948752957928719348759");
    print("Encrypted: $encrypted");

    String decrypted = secureComm.decrypt(encrypted);
    print("Decrypted: $decrypted");

  }

  do {

    stcp.StcpClient theClient =  stcp.StcpClient(serverHostname, serverPort, aes);
    
    await theClient.connect();

    theClient.recvStream().listen((String thedata) {
      logi.traceMe("[INCOMING Plain payload] $thedata");
    });

    int msg = 0;
    do {
      logi.traceMe("=================================================================================================");
      logi.traceMe("=================================================================================================");
      logi.traceMe("=================================================================================================");
      String out = "[$msg message]: $txt";
      theClient.send(out);
      logi.traceMe("Waiting $sendingIntervalMS ms......");
      await Future.delayed(Duration(milliseconds: sendingIntervalMS));
    } while (doLooping);

  } while (doLooping);
}
