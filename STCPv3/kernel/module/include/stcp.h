#pragma once

#define AF_STCP 45
#define PF_STCP AF_STCP

#define STCP_PROTO_DEFAULT 0
#define STCP_PROTO_TCP     253
#define STCP_PROTO_UDP     254

#define SOL_STCP 45
#define STCP_SO_COMPRESSION            1
#define STCP_SO_COMPRESSION_THRESHOLD  2
#define STCP_SO_COMPRESSION_STATS      3
#define STCP_COMPRESSION_DEFAULT_THRESHOLD 1024U

struct stcp_compression_stats {
    unsigned long long tx_attempts;
    unsigned long long tx_compressed_frames;
    unsigned long long tx_fallback_frames;
    unsigned long long tx_input_bytes;
    unsigned long long tx_wire_bytes;
    unsigned long long tx_errors;
    unsigned long long rx_compressed_frames;
    unsigned long long rx_wire_bytes;
    unsigned long long rx_output_bytes;
    unsigned long long rx_errors;
};
