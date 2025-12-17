#pragma once

#define ON  1
#define OFF 0
#define onoff(val) (val) ? "ON" : "OFF"

// Debugging
#define USE_OWN_SEND_MSG                    ON
#define USE_OWN_RECV_MSG                    ON
#define USE_OWN_DESTROY                     ON
// Jäädytetty
#define USE_OWN_PROT_OPTS                   ON
#define USE_OWN_SOCK_OPTS                   OFF
#define FORCE_TCP_PROTO                     OFF
#define USE_OWN_BIND                        OFF
#define DELEGATE_BIND                       OFF
#define USE_OWN_LISTEN                      OFF
#define DELEGATE_LISTEN                     OFF
#define USE_OWN_ACCEPT                      ON
#define USE_OWN_CONNECT                     ON
#define USE_OWN_RELEASE                     ON
#define ENABLE_PROTO_SWAPPING               ON
#define ENABLE_PROTO_FORCING                ON


#define STCP_IS_MAGIC_CHK_IF_DEAD           ON
#define STCP_IS_MAGIC_CHK_IF_NOT_ALIVE      ON 

