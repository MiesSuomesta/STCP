#pragma once

#define STCP_CONNECT_TO_ADDRESS_HOSTNAME    CONFIG_STCP_CONNECT_TO_HOST
#define STCP_CONNECT_TO_ADDRESS_PORT        CONFIG_STCP_CONNECT_TO_PORT

// The APN for SIM, consult your ISP
//#define STCP_LTE_APN                        CONFIG_STCP_LTE_APN_NAME
#define STCP_LTE_APN                        "internet"

#define STCP_LTE_SET_AT_CMD                  "AT+CGDCONT=1,\"IP\",\"" STCP_LTE_APN "\""

// Trackers
#define STCP_API_CONTEXT_TRACKING       0
#define STCP_POINTER_TRACKING           0
#define STCP_REF_COUNT_TRACKING         0
#define STCP_SOCKET_TRACKING            0
#define STCP_STCP_FSM_TRACKING          0


#define STCP_SOCKET_TRACKING_VERBOSE    0

#define STCP_DEBUG_ON_SELECTED_FILES    0


#define STCP_FSM_VERBOSE    0

