/*
 * SPDX-FileCopyrightText: Copyright The TrustedFirmware-M Contributors
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __TFM_VERSION_H__
#define __TFM_VERSION_H__

/*
 * Defines for TFM version.
 */
#define TFM_VERSION        2.3.0
#define TFM_VERSION_FULL   v2.3.0**

#define VERSTR(x)          #x
#define VERCON(x)          VERSTR(x)

#define VERSION_STR        VERCON(TFM_VERSION)
#define VERSION_FULLSTR    VERCON(TFM_VERSION_FULL)

/*
 * Define for the build timestamp string
 */
#define BUILD_TIMESTAMP "Mon 20 Jul 2026 09:50:14"

#endif /* __TFM_VERSION_H__ */
