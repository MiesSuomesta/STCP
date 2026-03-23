#pragma once

#define ON  1
#define OFF 0
#define onoff(val) (val) ? "ON" : "OFF"

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#define USE_ALIVE_CONTEXT_COUNTERS                  OFF

// Debugging
#define USE_OWN_SEND_MSG                            ON
#define USE_OWN_RECV_MSG                            ON
#define USE_OWN_DESTROY                             ON
// Jäädytetty
#define USE_OWN_PROT_OPTS                           OFF
#define USE_OWN_SOCK_OPTS                           ON

#define FORCE_TCP_PROTO                             OFF
#define USE_OWN_BIND                                ON
#define DELEGATE_BIND                               OFF
#define USE_OWN_LISTEN                              OFF
#define DELEGATE_LISTEN                             OFF
#define USE_OWN_ACCEPT                              ON
#define USE_OWN_CONNECT                             ON
#define USE_OWN_RELEASE                             ON


#define ENABLE_RATELIMIT_PRINTK                     OFF
#define STCP_IS_MAGIC_CHK_IF_DEAD                   OFF
#define STCP_IS_MAGIC_CHK_IF_NOT_ALIVE              OFF

#define WARN_ON_MAGIC_FAILURE                       OFF
#define WARN_ON_DOUBLE_FREE                         ON

#define STCP_WAIT_FOR_CONNECTION_ESTABLISHED_AT_IO  ON

// Timeouts
#define STCP_WAIT_FOR_HANDSHAKE_TO_COMPLETE_MSEC    15000
#define STCP_WAIT_FOR_TCP_ESTABLISHED_MSEC          15000

// Tilakoneen tilavaihoksia maksimissaan
#define STCP_HANDSHAKE_STATUS_MAX_PUMPS             32

// not in USE
#define ENABLE_PROTO_SWAPPING                       OFF
#define ENABLE_PROTO_FORCING                        OFF

#define STCP_SOCKET_BYPASS_ALL_IO                   ON