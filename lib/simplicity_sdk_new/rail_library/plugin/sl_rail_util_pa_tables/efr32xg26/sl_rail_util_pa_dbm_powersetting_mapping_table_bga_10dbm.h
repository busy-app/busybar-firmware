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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG26_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG26_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (45U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (100U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-337)

#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x0 /* est_deci-dBm:-337 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-327 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-317 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-307 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-297 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-287 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-277 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-267 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-257 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-247 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-237 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-227 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-217 act_deci-dBm:-337*/, \
    0x0 /* est_deci-dBm:-207 act_deci-dBm:-337*/, \
    0x1 /* est_deci-dBm:-197 act_deci-dBm:-204*/, \
    0x1 /* est_deci-dBm:-187 act_deci-dBm:-204*/, \
    0x1 /* est_deci-dBm:-177 act_deci-dBm:-204*/, \
    0x1 /* est_deci-dBm:-167 act_deci-dBm:-204*/, \
    0x1 /* est_deci-dBm:-157 act_deci-dBm:-204*/, \
    0x1 /* est_deci-dBm:-147 act_deci-dBm:-204*/, \
    0x2 /* est_deci-dBm:-137 act_deci-dBm:-144*/, \
    0x2 /* est_deci-dBm:-127 act_deci-dBm:-144*/, \
    0x2 /* est_deci-dBm:-117 act_deci-dBm:-144*/, \
    0x3 /* est_deci-dBm:-107 act_deci-dBm:-110*/, \
    0x3 /* est_deci-dBm:-97 act_deci-dBm:-110*/,  \
    0x3 /* est_deci-dBm:-87 act_deci-dBm:-110*/,  \
    0x4 /* est_deci-dBm:-77 act_deci-dBm:-86*/,   \
    0x5 /* est_deci-dBm:-67 act_deci-dBm:-69*/,   \
    0x5 /* est_deci-dBm:-57 act_deci-dBm:-69*/,   \
    0x6 /* est_deci-dBm:-47 act_deci-dBm:-55*/,   \
    0x7 /* est_deci-dBm:-37 act_deci-dBm:-42*/,   \
    0x8 /* est_deci-dBm:-27 act_deci-dBm:-32*/,   \
    0x9 /* est_deci-dBm:-17 act_deci-dBm:-23*/,   \
    0xb /* est_deci-dBm:-7 act_deci-dBm:-8*/,     \
    0xc /* est_deci-dBm:3 act_deci-dBm:-1*/,      \
    0xe /* est_deci-dBm:13 act_deci-dBm:9*/,      \
    0x10 /* est_deci-dBm:23 act_deci-dBm:19*/,    \
    0x13 /* est_deci-dBm:33 act_deci-dBm:33*/,    \
    0x16 /* est_deci-dBm:43 act_deci-dBm:43*/,    \
    0x1b /* est_deci-dBm:53 act_deci-dBm:52*/,    \
    0x1f /* est_deci-dBm:63 act_deci-dBm:62*/,    \
    0x26 /* est_deci-dBm:73 act_deci-dBm:73*/,    \
    0x30 /* est_deci-dBm:83 act_deci-dBm:83*/,    \
    0x3d /* est_deci-dBm:93 act_deci-dBm:93*/,    \
    0x4f /* est_deci-dBm:100 act_deci-dBm:100*/,  \
  }

#ifdef __cplusplus
}
#endif

#endif
