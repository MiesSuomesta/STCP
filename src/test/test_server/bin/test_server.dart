import 'dart:io' as os;

import 'package:args/args.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:stcp/stcp.dart' as stcp;

void main(List<String> arguments) async {
  // Luo ArgParser-olio
  final parser = ArgParser();

  String serverHostname = "localhost";
  int serverPort = 8888;
  String txt = "Testing string of data 1234567890" ;
  String aes = "Insecure? But secure :D";
  bool doLooping = false;
  bool doHelp = false;
  stcp.StcpCommon theStcpCommon = stcp.StcpCommon("nullkey");



  // Lisää argumentteja parseriin
  parser.addOption('host', help: 'server hostname');
  parser.addOption('port', help: 'server port');
  parser.addOption('txt', help: 'Text to send');
  parser.addOption('aes', help: 'AES encryption key, must be same at server instance');
  parser.addFlag  ('loop', help: 'Enable looping');
  parser.addFlag  ('help', help: 'Usage');

  // Parsitaan argumentit
  try {
    
    ArgResults argResults = parser.parse(arguments);
    serverHostname = argResults['host'] ?? "localhost";
    serverPort = int.parse(argResults['port'] ?? "8888");
    txt = argResults['txt'] ?? "Testing string of data 1234567890" ;

    String keyString = theStcpCommon.theAesKey; // 32 tavua
    aes = argResults['aes'] ?? keyString;

    doLooping = argResults['loop'];
    doHelp = argResults['help'];

  } catch (err) {
    print('Usage:');
    print(parser.usage);
  }

  if (doHelp) {
    print('Usage:');
    print(parser.usage);
    os.exit(0);
  }



  do {
    try {
      stcp.StcpServer theServer = stcp.StcpServer(serverHostname, serverPort, aes, theMessageHandler);
      logi.traceMe("Starting STCP server at $serverHostname:$serverPort ...");
      await theServer.start();
      logi.traceMe("STCP server exit .. doing loop: $doLooping");
    } catch (err) {
      logi.traceMe("Server got error: $err");
    }
    logi.traceMe("Waiting 1500 ms......");
    await Future.delayed(Duration(milliseconds: 1500));
  } while (doLooping);
}

String theMessageHandler(String msgIn) {
  String output = msgIn.toUpperCase();
  logi.traceMe("MSG in : $msgIn");
  logi.traceMe("MSG out: $output");
  return output;
}
