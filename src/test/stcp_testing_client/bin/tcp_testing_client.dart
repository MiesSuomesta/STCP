import 'dart:convert';
import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:args/args.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:utils/utils.dart' as util;
import 'package:the_secure_comm/the_secure_comm.dart' as scomm;
import 'package:stcp/stcp.dart' as stcp;

const String version = '0.0.1';

ArgParser buildParser() {
  return ArgParser()
    ..addFlag(
      'help',
      abbr: 'h',
      negatable: false,
      help: 'Print this usage information.',
    )
    ..addFlag(
      'verbose',
      abbr: 'v',
      negatable: false,
      help: 'Show additional command output.',
    )
    ..addFlag(
      'version',
      negatable: false,
      help: 'Print the tool version.',
    )
    ..addFlag(
      'port',
      negatable: false,
      help: 'Ther port to connect to.',
    )
    ..addFlag(
      'service',
      negatable: false,
      help: 'Ther port to connect to.',
    )
    ..addFlag(
      'class',
      negatable: false,
      help: 'Ther port to connect to.',
    )
    ..addFlag(
      'payload',
      negatable: false,
      help: 'Ther port to connect to.',
    )
    ..addFlag(
      'loop',
      negatable: false,
      help: 'Wether to loop or not.',
    )
    ..addFlag(
      'check-scomm',
      negatable: false,
      help: 'Wether to loop or not.',
    )
    ..addFlag(
      'check-msg-crypt',
      negatable: false,
      help: 'Wether to loop or not.',
    )
    ..addFlag(
      'interval',
      negatable: false,
      help: 'Wait for X ms per loop.',
    )
    ..addFlag(
      'host',
      negatable: false,
      help: 'The hostname to connect to.',
    );
}

void printUsage(ArgParser argParser) {
  print('Usage: dart stcp_testing_client.dart <flags> [arguments]');
  print(argParser.usage);
}

void startTolisten(Stream striimi) async {
  logi.traceMe("Starting to listen....");
  striimi.listen((data) {
    logi.traceMe("Got input from server: ${String.fromCharCodes(data)}");
  }).onDone(() {
    logi.traceMe("Socket disconnected...");
  });
  logi.traceMe("Past....");
}

void main(List<String> arguments) async {
  final ArgParser argParser = buildParser();
  String host = "localhost";
  int port = 8080;

  String strService = "serviisi";
  String strClass = "luokka";
  String strPayload = "dataa";

  bool doLoop = false;
  bool doCheckSCOMM = false;
  bool doCheckMsgCrypt = false;

  try {
    final ArgResults results = argParser.parse(arguments);
    bool verbose = false;

    // Process the parsed arguments.
    if (results.flag('help')) {
      printUsage(argParser);
      return;
    }

    if (results.flag('version')) {
      print('stcp_testing_client version: $version');
      return;
    }

    if (results.flag('verbose')) {
      verbose = true;
    }

    if (results.flag('host')) {
      host = results["host"];
    }

    if (results.flag('port')) {
      port = results["port"];
    }

    if (results.flag('service')) {
      strService = results["service"];
    }

    if (results.flag('class')) {
      strClass = results["class"];
    }

    if (results.flag('payload')) {
      strPayload = results["payload"];
    }

    if (results.flag('loop')) {
      doLoop = true;
    }

    if (results.flag('check-scomm')) {
      doCheckSCOMM = true;
    }

    if (results.flag('check-msg-crypt')) {
      doCheckMsgCrypt = true;
    }
  } on FormatException catch (e) {
    // Print usage information if an invalid argument was provided.
    print(e.message);
    print('');
    printUsage(argParser);
  }

  String data = "Hello World!";
  List<int> dataIntLst = data.codeUnits;
  Uint8List dataUILst = Uint8List.fromList(dataIntLst);
  String theSharedKey = "ABCDEFG0000000000000000012345612";
  Uint8List theSharedKeyUintList = Uint8List.fromList(theSharedKey.codeUnits);
  bool exitShort = false;
  if (doCheckMsgCrypt) {
    exitShort = true;
    logi.traceMe("Doing message crypting check");

    final theSCommon = stcp.StcpCommon();

    final encrypted = theSCommon.theSecureMessageTransferOutgoing(
        dataUILst, theSharedKeyUintList);
    logi.traceMe("Got encrypted: $encrypted");

    final decrypted = theSCommon.theSecureMessageTransferIncoming(
        encrypted, theSharedKeyUintList);
    logi.traceMe("Got decrypted: $decrypted");
  }

  if (doCheckSCOMM) {
    exitShort = true;
    logi.traceMe("Doing scomm check");

    String data = "Hello World!";
    scomm.SecureCommunication theSC = scomm.SecureCommunication(
      theSharedKey,
      "1234567890123456",
      "Koe koe",
    );

    final encrypted = theSC.encrypt(data);
    logi.traceMe("Got encrypted: $encrypted");
    final decrypted = theSC.decrypt(encrypted);
    logi.traceMe("Got decrypted: $decrypted");
  }

  if (exitShort) {
    exit(0);
  }

  // Main startti
  stcp.StcpClient theClient = stcp.StcpClient(host, port, theMessageHandler);
  await theClient.connect();

  logi.traceMe("Continuing to looop, loop: $doLoop");
  bool once = true;
  while (doLoop || once) {
    try {
      /*
      tc.TcpClient theClient =
          tc.TcpClient(host, port, "1234567890123456", theMessageHandler);
      */
      int msg = 0;
      logi.traceMe("Mainissa jatkuu whileen..");
      while (doLoop || once) {
        msg += 1;
        String msgout = "Viesti numero $msg";
        logi.traceMe("Sending $msgout as utf8 encoded...");
        Uint8List msgList = Uint8List.fromList(msgout.codeUnits);
        theClient.stcp_send(msgList);
        Uint8List? tmp = await theClient.stcp_recv();
        logi.traceIntList("Received to client", tmp);
        await Future.delayed(Duration(seconds: 1));
        once = false;
      }
    } catch (err) {
      logi.traceMe("Got error $err // ${theClient}");
      theClient.stop();
      theClient = stcp.StcpClient(host, port, theMessageHandler);
      await theClient.connect();
    }
    await Future.delayed(Duration(seconds: 5));
  }
  logi.traceMe("quittaus....");
}

Future<Uint8List?> theMessageHandler(Socket sck, Uint8List dataIn) async {
  String msgIn = String.fromCharCodes(dataIn.toList());
  logi.traceMe("Got incomin message: ${msgIn}");
  String out = "Serveriltä tuli: $msgIn";
  return Uint8List.fromList(out.codeUnits);
}
