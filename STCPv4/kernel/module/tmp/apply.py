#!/usr/bin/env python3
from pathlib import Path
import sys

p = Path('src/stcp_carrier.c')
if not p.exists():
    sys.exit('ERROR: run from STCPv4/kernel/module (src/stcp_carrier.c not found)')
s = p.read_text()
if 'C:CARRIER_TCP_TX:KERNEL_SEND' in s:
    print('OK: carrier TX benchmark bracketing already present')
    sys.exit(0)

# Make this overlay standalone with respect to declaration order.
anchor = '''static void stcp_carrier_record_terminal_error(\n\tstruct stcp_carrier *carrier,\n\tint error\n);\n'''
proto = anchor + '''\nextern u64 stcp_kernel_benchmark_now_ns(void);\nextern void stcp_kernel_benchmark_record(\n\tconst u8 *file, size_t file_len, u32 line, u32 column, u64 elapsed_ns\n);\n'''
if 'extern u64 stcp_kernel_benchmark_now_ns(void);' not in s:
    if anchor not in s:
        sys.exit('ERROR: prototype insertion anchor not found')
    s = s.replace(anchor, proto, 1)

# UDP control: bracket the actual kernel_sendmsg only.
old = '''\tret = kernel_sendmsg(root->socket, &message, &vector, 1, len);\n\tif (ret >= 0 && (size_t)ret != len)\n'''
new = '''\t{\n\t\tu64 bench_udp_tx_kernel = stcp_kernel_benchmark_now_ns();\n\t\tret = kernel_sendmsg(root->socket, &message, &vector, 1, len);\n\t\tstcp_kernel_benchmark_record((const u8 *)"C:CARRIER_UDP_TX:KERNEL_SEND",\n\t\t\tsizeof("C:CARRIER_UDP_TX:KERNEL_SEND") - 1, 0, 0,\n\t\t\tstcp_kernel_benchmark_now_ns() - bench_udp_tx_kernel);\n\t}\n\tif (ret >= 0 && (size_t)ret != len)\n'''
if old not in s:
    sys.exit('ERROR: UDP kernel_sendmsg anchor not found')
s = s.replace(old, new, 1)

# TCP: total begins before lifecycle lock, so lock contention is included.
old = '''\t{\n\t\tstruct socket *send_socket;\n\t\tbool trace_send = atomic_dec_if_positive(&carrier->debug_tx_budget) >= 0;\n\n\t\tif (trace_send)\n'''
new = '''\t{\n\t\tstruct socket *send_socket;\n\t\tu64 bench_tcp_tx_total = stcp_kernel_benchmark_now_ns();\n\t\tu64 bench_tcp_tx_lock;\n\t\tbool trace_send = atomic_dec_if_positive(&carrier->debug_tx_budget) >= 0;\n\n\t\tif (trace_send)\n'''
if old not in s:
    sys.exit('ERROR: TCP send block anchor not found')
s = s.replace(old, new, 1)

old = '''\t\tmutex_lock(&carrier->lifecycle_lock);\n\t\tif (carrier->stopping || !carrier->socket || !carrier->connected) {\n'''
new = '''\t\tbench_tcp_tx_lock = stcp_kernel_benchmark_now_ns();\n\t\tmutex_lock(&carrier->lifecycle_lock);\n\t\tstcp_kernel_benchmark_record((const u8 *)"C:CARRIER_TCP_TX:LIFECYCLE_LOCK_WAIT",\n\t\t\tsizeof("C:CARRIER_TCP_TX:LIFECYCLE_LOCK_WAIT") - 1, 0, 0,\n\t\t\tstcp_kernel_benchmark_now_ns() - bench_tcp_tx_lock);\n\t\tif (carrier->stopping || !carrier->socket || !carrier->connected) {\n'''
# only replace occurrence in TCP block: use rfind-ish after marker
idx = s.find('u64 bench_tcp_tx_total')
pos = s.find(old, idx)
if pos < 0:
    sys.exit('ERROR: TCP lifecycle lock anchor not found')
s = s[:pos] + s[pos:].replace(old, new, 1)

old = '''\t\tret = kernel_sendmsg(\n\t\t\tsend_socket,\n\t\t\t&message,\n\t\t\t&vector,\n\t\t\t1,\n\t\t\tlen - position\n\t\t);\n'''
new = '''\t\t{\n\t\t\tu64 bench_tcp_tx_kernel = stcp_kernel_benchmark_now_ns();\n\t\t\tret = kernel_sendmsg(\n\t\t\t\tsend_socket,\n\t\t\t\t&message,\n\t\t\t\t&vector,\n\t\t\t\t1,\n\t\t\t\tlen - position\n\t\t\t);\n\t\t\tstcp_kernel_benchmark_record((const u8 *)"C:CARRIER_TCP_TX:KERNEL_SEND",\n\t\t\t\tsizeof("C:CARRIER_TCP_TX:KERNEL_SEND") - 1, 0, 0,\n\t\t\t\tstcp_kernel_benchmark_now_ns() - bench_tcp_tx_kernel);\n\t\t}\n'''
if old not in s:
    sys.exit('ERROR: TCP kernel_sendmsg anchor not found')
s = s.replace(old, new, 1)

# Record successful/error normal completion. Early ESHUTDOWN before active_sends is intentionally excluded.
old = '''\t\tif (trace_send)\n\t\tpr_emerg("stcp-xconnect: TX05 send-done cid=%llu carrier=%px result=%zd bytes=%zu active=%d\\n",\n\t\t\t READ_ONCE(carrier->debug_id), carrier, send_result, position,\n\t\t\t atomic_read(&carrier->active_sends));\n\t}\n\n\tif (send_result < 0) {\n'''
new = '''\t\tif (trace_send)\n\t\tpr_emerg("stcp-xconnect: TX05 send-done cid=%llu carrier=%px result=%zd bytes=%zu active=%d\\n",\n\t\t\t READ_ONCE(carrier->debug_id), carrier, send_result, position,\n\t\t\t atomic_read(&carrier->active_sends));\n\t\tstcp_kernel_benchmark_record((const u8 *)"C:CARRIER_TCP_TX:TOTAL",\n\t\t\tsizeof("C:CARRIER_TCP_TX:TOTAL") - 1, 0, 0,\n\t\t\tstcp_kernel_benchmark_now_ns() - bench_tcp_tx_total);\n\t}\n\n\tif (send_result < 0) {\n'''
if old not in s:
    sys.exit('ERROR: TCP total completion anchor not found')
s = s.replace(old, new, 1)

p.write_text(s)
print('OK: carrier TCP/UDP TX benchmark bracketing applied to src/stcp_carrier.c')
