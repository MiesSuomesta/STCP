/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __OS_WRAPPER_KERNEL_H__
#define __OS_WRAPPER_KERNEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "os_wrapper/common.h"

/**
 * \brief Returns value that indicates whether RTOS kernel has started running.
 *
 * \return true if RTOS kernel has been started.
 *         false if RTOS kernel has not been started.
 */
bool os_wrapper_is_kernel_started(void);

#ifdef __cplusplus
}
#endif

#endif /* __OS_WRAPPER_KERNEL_H__ */
