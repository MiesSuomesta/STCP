import 'dart:convert';
import 'dart:io';
import 'dart:core';
import 'dart:async';
import 'dart:typed_data';

import 'package:paxlog/paxlog.dart' as logi;
import 'package:stcp/src/stcpCommon.dart';
import 'package:utils/utils.dart' as utils;

import 'dart:typed_data';

class StcpHeader {
  static const int TAG = 0xDEADBEEF;
  static const int TAG_SIZE = 4;
  static const int LEN_SIZE = 8;
  static const int TOTAL_SIZE = TAG_SIZE + LEN_SIZE;
  static const int MAX_MESSAGE_SIZE = 1 << 28; // 128 MiB

  int length;

  StcpHeader(this.length) {
    if (length < 0 || length > MAX_MESSAGE_SIZE) {
      throw ArgumentError("Invalid message length: $length");
    }
  }

  static Uint8List createTagBytes() {
    final tag = ByteData(STCP_HEADER_TAG_SIZE);
    tag.setUint32(0, STCP_TAG, Endian.big);
    return tag.buffer.asUint8List();
  }

  static bool doesTagMatch(Uint8List tagIn) {
    if (tagIn.length < STCP_HEADER_TOTAL_SIZE) return false;

    Uint8List theTAG = createTagBytes();
    if (theTAG.length != tagIn.length) {
      return false;
    }

    int idx = 0;
    while (idx < theTAG.length) {
      if (theTAG[idx] != tagIn[idx]) {
        return false;
      }
      idx++;
    }
    return true;
  }

  static bool isProbablyHeadered(Uint8List data) {
    if (data.length < STCP_HEADER_TOTAL_SIZE) {
      logi.traceMeBetter("Not enough bytes to check.");
      return false;
    }

    Uint8List theTAG = createTagBytes();
    Uint8List theTAGincoming = data.sublist(0, STCP_HEADER_TAG_SIZE);
    if (doesTagMatch(theTAGincoming)) {
      logi.traceMeIf(false, "Header TAG found");
      return true;
    }
    //logi.traceMeBetter("RE-doing headers from stack.");
    return false;
  }

  static (int?, StcpHeader?) tryParseHeaderInBuffer(Uint8List buffer) {
    const int headerLen = TOTAL_SIZE;
    if (buffer.length < headerLen) {
      logi.traceMeIf(false, "Not enough bytes! ${buffer.length} bytes given.");
      return (null, null);
    }

    final bd = ByteData.sublistView(buffer);
    for (int i = 0; i <= buffer.length - headerLen; i++) {
      logi.traceMeIf(false, "Checking buffer at index $i ..");
      final tag = bd.getUint32(i, Endian.big);
      logi.traceMeIf(false, "Parsed tag: $tag");
      if (tag != TAG) continue;

      final len = bd.getInt64(i + TAG_SIZE, Endian.big);
      final bool lenOK = len >= 0 && len < MAX_MESSAGE_SIZE;
      logi.traceMeIf(false, "Parsed len OK:($lenOK): $len");

      if (!lenOK) continue;

      logi.traceMeIf(false,
          "Found header candidate at offset $i: tag=0x${tag.toRadixString(16)}, len=$len");

      try {
        final hdr = StcpHeader(len);
        return (i, hdr);
      } catch (_) {
        continue;
      }
    }

    return (null, null);
  }

  static Uint8List create(int payloadLength) {
    final buffer = ByteData(STCP_HEADER_TOTAL_SIZE);
    buffer.setUint32(0, STCP_TAG, Endian.big); // Magic
    buffer.setInt64(STCP_HEADER_TAG_SIZE, payloadLength,
        Endian.big); // Payload length (int64)
    return buffer.buffer.asUint8List();
  }

  static Uint8List stripHeader(Uint8List fullPacket) {
    final len =
        StcpHeader.parseLength(fullPacket.sublist(0, StcpHeader.TOTAL_SIZE));
    return fullPacket.sublist(
        StcpHeader.TOTAL_SIZE, StcpHeader.TOTAL_SIZE + len);
  }

  static int parseLength(Uint8List header) {
    if (header.length < TOTAL_SIZE) {
      throw Exception("STCPHeader.parseLength: Header too short!");
    }

    final reader = ByteData.sublistView(header);
    final magicValue = reader.getUint32(0, Endian.big);

    if (magicValue != STCP_TAG) {
      throw Exception(
          "Invalid STCP header: expected 0xdeadbeef, got 0x${magicValue.toRadixString(16)}");
    }

    return reader.getInt64(STCP_HEADER_TAG_SIZE, Endian.big);
  }

  /// Luo headeriksi Uint8Listin lähetettäväksi
  Uint8List toBytes() {
    final bd = ByteData(TOTAL_SIZE);
    bd.setUint32(0, TAG, Endian.big);
    bd.setInt64(STCP_HEADER_TAG_SIZE, length, Endian.big);
    final out = bd.buffer.asUint8List();
    logi.traceIntListIf(false, "Header out", out);
    return out;
  }

  /// Parsii headerin bytemuodosta
  static StcpHeader? fromBytes(Uint8List bytes) {
    logi.traceIntListIf(false, "Incoming raw", bytes, noStr: true);

    if (bytes.length != TOTAL_SIZE) {
      logi.traceMeIf(false, "Not header, lenght mismatch");
      return null;
    }

    final bd = ByteData.sublistView(bytes);
    final tag = bd.getUint32(0, Endian.big);
    logi.traceMeIf(false,
        "Parsed tag: 0x${tag.toRadixString(16).padLeft(8, '0')} (expected: 0x${TAG.toRadixString(16)})");

    final len = bd.getInt64(TAG_SIZE, Endian.big);
    logi.traceMeIf(false, "Parsed len: $len");

    bool tagOK = tag == TAG;
    if (!tagOK) return null;

    bool lenOK = len >= 0 && len < MAX_MESSAGE_SIZE;
    logi.traceMeIf(false, "Got len check: ${lenOK}");
    if (!lenOK) return null;

    logi.traceMeIf(false, "Got header ok..");
    return StcpHeader(len);
  }

  /// Tarkistaa onko bytes validi STCP-headeri
  static bool isValid(Uint8List bytes) {
    return fromBytes(bytes) != null;
  }
}
