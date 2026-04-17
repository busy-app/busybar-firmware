/***************************************************************************//**
 * @file
 * @brief PA power conversion curves used by Silicon Labs PA power conversion
 *   functions.
 * @details This file contains the curves needed convert PA power levels to
 *   dBm powers.
 *******************************************************************************
 * # License
 * <b>Copyright 2025 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef __PA_DBM_POWERSETTING_MAPPING_SIXG301_H_
#define __PA_DBM_POWERSETTING_MAPPING_SIXG301_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (28U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (20U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-250)

#define SL_RAIL_PA_TABLE_0  \
  {                         \
    0x8000 /* -25.0 dBm */, \
    0x8000 /* -24.0 dBm */, \
    0x8000 /* -23.0 dBm */, \
    0x8000 /* -22.0 dBm */, \
    0x8000 /* -21.0 dBm */, \
    0x8001 /* -20.0 dBm */, \
    0x8001 /* -19.0 dBm */, \
    0x8001 /* -18.0 dBm */, \
    0x8001 /* -17.0 dBm */, \
    0x8001 /* -16.0 dBm */, \
    0x8002 /* -15.0 dBm */, \
    0x8002 /* -14.0 dBm */, \
    0x8002 /* -13.0 dBm */, \
    0x8003 /* -12.0 dBm */, \
    0x8003 /* -11.0 dBm */, \
    0x8003 /* -10.0 dBm */, \
    0x8004 /* -9.0 dBm */,  \
    0x8005 /* -8.0 dBm */,  \
    0x8005 /* -7.0 dBm */,  \
    0x8006 /* -6.0 dBm */,  \
    0x8007 /* -5.0 dBm */,  \
    0x8009 /* -4.0 dBm */,  \
    0x800b /* -3.0 dBm */,  \
    0x800d /* -2.0 dBm */,  \
    0x8010 /* -1.0 dBm */,  \
    0x8014 /* 0.0 dBm */,   \
    0x801b /* 1.0 dBm */,   \
    0x801f /* 2.0 dBm */    \
  }

#endif

#ifdef __cplusplus
}
#endif
