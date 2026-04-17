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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG21_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG21_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (41U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (200U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-197)

#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x1 /* est_deci-dBm:-197 act_deci-dBm:-197*/, \
    0x1 /* est_deci-dBm:-187 act_deci-dBm:-197*/, \
    0x1 /* est_deci-dBm:-177 act_deci-dBm:-197*/, \
    0x1 /* est_deci-dBm:-167 act_deci-dBm:-197*/, \
    0x1 /* est_deci-dBm:-157 act_deci-dBm:-197*/, \
    0x1 /* est_deci-dBm:-147 act_deci-dBm:-197*/, \
    0x2 /* est_deci-dBm:-137 act_deci-dBm:-137*/, \
    0x2 /* est_deci-dBm:-127 act_deci-dBm:-137*/, \
    0x2 /* est_deci-dBm:-117 act_deci-dBm:-137*/, \
    0x2 /* est_deci-dBm:-107 act_deci-dBm:-137*/, \
    0x3 /* est_deci-dBm:-97 act_deci-dBm:-103*/,  \
    0x3 /* est_deci-dBm:-87 act_deci-dBm:-103*/,  \
    0x4 /* est_deci-dBm:-77 act_deci-dBm:-78*/,   \
    0x4 /* est_deci-dBm:-67 act_deci-dBm:-78*/,   \
    0x5 /* est_deci-dBm:-57 act_deci-dBm:-59*/,   \
    0x5 /* est_deci-dBm:-47 act_deci-dBm:-59*/,   \
    0x6 /* est_deci-dBm:-37 act_deci-dBm:-44*/,   \
    0x7 /* est_deci-dBm:-27 act_deci-dBm:-32*/,   \
    0x8 /* est_deci-dBm:-17 act_deci-dBm:-20*/,   \
    0x9 /* est_deci-dBm:-7 act_deci-dBm:-11*/,    \
    0xa /* est_deci-dBm:3 act_deci-dBm:-2*/,      \
    0xc /* est_deci-dBm:13 act_deci-dBm:13*/,     \
    0xd /* est_deci-dBm:23 act_deci-dBm:19*/,     \
    0xf /* est_deci-dBm:33 act_deci-dBm:30*/,     \
    0x11 /* est_deci-dBm:43 act_deci-dBm:41*/,    \
    0x13 /* est_deci-dBm:53 act_deci-dBm:51*/,    \
    0x16 /* est_deci-dBm:63 act_deci-dBm:63*/,    \
    0x19 /* est_deci-dBm:73 act_deci-dBm:73*/,    \
    0x1c /* est_deci-dBm:83 act_deci-dBm:82*/,    \
    0x20 /* est_deci-dBm:93 act_deci-dBm:93*/,    \
    0x24 /* est_deci-dBm:103 act_deci-dBm:103*/,  \
    0x29 /* est_deci-dBm:113 act_deci-dBm:113*/,  \
    0x2c /* est_deci-dBm:123 act_deci-dBm:120*/,  \
    0x34 /* est_deci-dBm:133 act_deci-dBm:132*/,  \
    0x3b /* est_deci-dBm:143 act_deci-dBm:142*/,  \
    0x43 /* est_deci-dBm:153 act_deci-dBm:153*/,  \
    0x4e /* est_deci-dBm:163 act_deci-dBm:163*/,  \
    0x59 /* est_deci-dBm:173 act_deci-dBm:172*/,  \
    0x6a /* est_deci-dBm:183 act_deci-dBm:183*/,  \
    0x82 /* est_deci-dBm:193 act_deci-dBm:193*/,  \
    0x97 /* est_deci-dBm:200 act_deci-dBm:200*/,  \
  }

#ifdef __cplusplus
}
#endif

#endif
