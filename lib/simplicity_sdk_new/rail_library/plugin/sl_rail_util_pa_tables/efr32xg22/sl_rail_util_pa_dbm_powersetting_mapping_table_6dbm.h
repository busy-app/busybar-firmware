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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG22_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG22_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (37U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (85U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-275)

#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x0 /* est_deci-dBm:-275 act_deci-dBm:-275*/, \
    0x0 /* est_deci-dBm:-265 act_deci-dBm:-275*/, \
    0x0 /* est_deci-dBm:-255 act_deci-dBm:-275*/, \
    0x0 /* est_deci-dBm:-245 act_deci-dBm:-275*/, \
    0x0 /* est_deci-dBm:-235 act_deci-dBm:-275*/, \
    0x0 /* est_deci-dBm:-225 act_deci-dBm:-275*/, \
    0x0 /* est_deci-dBm:-215 act_deci-dBm:-275*/, \
    0x1 /* est_deci-dBm:-205 act_deci-dBm:-212*/, \
    0x1 /* est_deci-dBm:-195 act_deci-dBm:-212*/, \
    0x1 /* est_deci-dBm:-185 act_deci-dBm:-212*/, \
    0x1 /* est_deci-dBm:-175 act_deci-dBm:-212*/, \
    0x1 /* est_deci-dBm:-165 act_deci-dBm:-212*/, \
    0x2 /* est_deci-dBm:-155 act_deci-dBm:-160*/, \
    0x2 /* est_deci-dBm:-145 act_deci-dBm:-160*/, \
    0x2 /* est_deci-dBm:-135 act_deci-dBm:-160*/, \
    0x3 /* est_deci-dBm:-125 act_deci-dBm:-127*/, \
    0x3 /* est_deci-dBm:-115 act_deci-dBm:-127*/, \
    0x3 /* est_deci-dBm:-105 act_deci-dBm:-127*/, \
    0x4 /* est_deci-dBm:-95 act_deci-dBm:-104*/,  \
    0x5 /* est_deci-dBm:-85 act_deci-dBm:-86*/,   \
    0x5 /* est_deci-dBm:-75 act_deci-dBm:-86*/,   \
    0x6 /* est_deci-dBm:-65 act_deci-dBm:-71*/,   \
    0x7 /* est_deci-dBm:-55 act_deci-dBm:-59*/,   \
    0x8 /* est_deci-dBm:-45 act_deci-dBm:-48*/,   \
    0x9 /* est_deci-dBm:-35 act_deci-dBm:-39*/,   \
    0xa /* est_deci-dBm:-25 act_deci-dBm:-31*/,   \
    0xc /* est_deci-dBm:-15 act_deci-dBm:-17*/,   \
    0xe /* est_deci-dBm:-5 act_deci-dBm:-6*/,     \
    0x10 /* est_deci-dBm:5 act_deci-dBm:3*/,      \
    0x13 /* est_deci-dBm:15 act_deci-dBm:15*/,    \
    0x16 /* est_deci-dBm:25 act_deci-dBm:24*/,    \
    0x1a /* est_deci-dBm:35 act_deci-dBm:34*/,    \
    0x21 /* est_deci-dBm:45 act_deci-dBm:45*/,    \
    0x28 /* est_deci-dBm:55 act_deci-dBm:54*/,    \
    0x34 /* est_deci-dBm:65 act_deci-dBm:65*/,    \
    0x49 /* est_deci-dBm:75 act_deci-dBm:75*/,    \
    0x79 /* est_deci-dBm:85 act_deci-dBm:85*/,    \
  }

#ifdef __cplusplus
}
#endif

#endif
