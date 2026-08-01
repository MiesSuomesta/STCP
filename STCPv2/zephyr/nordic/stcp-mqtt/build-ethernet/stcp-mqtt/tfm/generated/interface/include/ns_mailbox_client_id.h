/*
 * Copyright (c) 2024, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
/***********  WARNING: This is an auto-generated file. Do not edit!  ***********/
#ifndef __NS_MAILBOX_CLIENT_ID_H__
#define __NS_MAILBOX_CLIENT_ID_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/***********************************************************************************/
/* Defines for all the NS mailbox client id address ranges                         */
/***********************************************************************************/

/*******************************************************************************************/
/* Define a structure containing all the client id ranges and source information           */
/*******************************************************************************************/
struct tfm_ns_mailbox_client_id_info {
    uint32_t source;
    int32_t client_id_base;
    int32_t client_id_limit;
};


#ifdef __cplusplus
}
#endif

#endif /* __NS_MAILBOX_CLIENT_ID_H__ */