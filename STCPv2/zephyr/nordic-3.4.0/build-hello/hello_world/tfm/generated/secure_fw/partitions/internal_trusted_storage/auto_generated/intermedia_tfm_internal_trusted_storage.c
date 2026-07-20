/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

/***********  WARNING: This is an auto-generated file. Do not edit!  ***********/

#include <stdint.h>
#include "config_tfm.h"

#ifdef CONFIG_TFM_REUSE_COPY_AREA_FOR_SP_STACKS
__attribute__((section(".tfm_runtime_sp_stacks")))
#endif
uint8_t tfm_sp_its_stack[ITS_STACK_SIZE] __attribute__((aligned(8)));
