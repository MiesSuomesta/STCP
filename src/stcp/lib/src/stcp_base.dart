library stcp;

export './stcpCommon.dart';
export './stcpServer.dart';
export './stcpClient.dart';

typedef processMsgFunction  = String Function(String msgIn);
