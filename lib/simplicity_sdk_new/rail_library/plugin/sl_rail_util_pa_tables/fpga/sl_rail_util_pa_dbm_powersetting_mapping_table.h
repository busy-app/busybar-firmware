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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (45U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (120U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-320)

#define SL_RAIL_PA_TABLE_0 \
  {                        \
    0x0 /* -32.0 dBm */,   \
    0x0 /* -31.0 dBm */,   \
    0x0 /* -30.0 dBm */,   \
    0x0 /* -29.0 dBm */,   \
    0x0 /* -28.0 dBm */,   \
    0x0 /* -27.0 dBm */,   \
    0x0 /* -26.0 dBm */,   \
    0x0 /* -25.0 dBm */,   \
    0x0 /* -24.0 dBm */,   \
    0x0 /* -23.0 dBm */,   \
    0x0 /* -22.0 dBm */,   \
    0x0 /* -21.0 dBm */,   \
    0x0 /* -20.0 dBm */,   \
    0x0 /* -19.0 dBm */,   \
    0x0 /* -18.0 dBm */,   \
    0x0 /* -17.0 dBm */,   \
    0x0 /* -16.0 dBm */,   \
    0x0 /* -15.0 dBm */,   \
    0x1 /* -14.0 dBm */,   \
    0x1 /* -13.0 dBm */,   \
    0x1 /* -12.0 dBm */,   \
    0x1 /* -11.0 dBm */,   \
    0x1 /* -10.0 dBm */,   \
    0x1 /* -9.0 dBm */,    \
    0x2 /* -8.0 dBm */,    \
    0x2 /* -7.0 dBm */,    \
    0x2 /* -6.0 dBm */,    \
    0x3 /* -5.0 dBm */,    \
    0x3 /* -4.0 dBm */,    \
    0x3 /* -3.0 dBm */,    \
    0x4 /* -2.0 dBm */,    \
    0x5 /* -1.0 dBm */,    \
    0x5 /* 0.0 dBm */,     \
    0x6 /* 1.0 dBm */,     \
    0x7 /* 2.0 dBm */,     \
    0x8 /* 3.0 dBm */,     \
    0x9 /* 4.0 dBm */,     \
    0xb /* 5.0 dBm */,     \
    0xd /* 6.0 dBm */,     \
    0x10 /* 7.0 dBm */,    \
    0x13 /* 8.0 dBm */,    \
    0x19 /* 9.0 dBm */,    \
    0x22 /* 10.0 dBm */,   \
    0x37 /* 11.0 dBm */,   \
    0x5f /* 12.0 dBm */    \
  }

#endif

#ifdef __cplusplus
}
#endif
