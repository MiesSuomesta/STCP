import 'dart:convert';
import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:args/args.dart';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:utils/utils.dart' as util;
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
      'host',
      negatable: false,
      help: 'The hostname to connect to.',
    );
}

void printUsage(ArgParser argParser) {
  print('Usage: dart stcp_testing_client.dart <flags> [arguments]');
  print(argParser.usage);
}

void main(List<String> arguments) async {
  final ArgParser argParser = buildParser();
  String host = "localhost";
  int port = 8080;

  String strService = "serviisi";
  String strClass = "luokka";
  String strPayload = "dataa";

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
  } on FormatException catch (e) {
    // Print usage information if an invalid argument was provided.
    print(e.message);
    print('');
    printUsage(argParser);
  }

  // Main startti
  /*
  ts.TcpServer theServeri =
      ts.TcpServer(host, port, "1234567890123456", theMessageHandler);
      */
  stcp.StcpServer theServer = stcp.StcpServer(host, port, theMessageHandler);

  await theServer.start();
  while (true) {
    logi.traceMe("Server listening ...");
    await Future.delayed(Duration(seconds: 15));
  }
}

Future<Uint8List?> theMessageHandler(Socket sck, Uint8List dataIn) async {
  String msgIn = String.fromCharCodes(dataIn.toList());
  logi.traceMe("Got incomin message: ${msgIn}");

  String out = "Serveriltä kuittaus...";
  return Uint8List.fromList(out.codeUnits);
}
