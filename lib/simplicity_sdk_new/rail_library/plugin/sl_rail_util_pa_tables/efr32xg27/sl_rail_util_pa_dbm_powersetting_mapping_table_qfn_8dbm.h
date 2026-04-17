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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG27_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG27_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (39U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (80U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-300)

#define SL_RAIL_PA_TABLE_0                        \
  {                                               \
    0x0 /* est_deci-dBm:-300 act_deci-dBm:-309*/, \
    0x0 /* est_deci-dBm:-290 act_deci-dBm:-309*/, \
    0x0 /* est_deci-dBm:-280 act_deci-dBm:-309*/, \
    0x0 /* est_deci-dBm:-270 act_deci-dBm:-309*/, \
    0x0 /* est_deci-dBm:-260 act_deci-dBm:-309*/, \
    0x0 /* est_deci-dBm:-250 act_deci-dBm:-309*/, \
    0x1 /* est_deci-dBm:-240 act_deci-dBm:-240*/, \
    0x1 /* est_deci-dBm:-230 act_deci-dBm:-240*/, \
    0x1 /* est_deci-dBm:-220 act_deci-dBm:-240*/, \
    0x1 /* est_deci-dBm:-210 act_deci-dBm:-240*/, \
    0x1 /* est_deci-dBm:-200 act_deci-dBm:-240*/, \
    0x1 /* est_deci-dBm:-190 act_deci-dBm:-240*/, \
    0x2 /* est_deci-dBm:-180 act_deci-dBm:-187*/, \
    0x2 /* est_deci-dBm:-170 act_deci-dBm:-187*/, \
    0x2 /* est_deci-dBm:-160 act_deci-dBm:-187*/, \
    0x3 /* est_deci-dBm:-150 act_deci-dBm:-159*/, \
    0x3 /* est_deci-dBm:-140 act_deci-dBm:-159*/, \
    0x4 /* est_deci-dBm:-130 act_deci-dBm:-135*/, \
    0x4 /* est_deci-dBm:-120 act_deci-dBm:-135*/, \
    0x5 /* est_deci-dBm:-110 act_deci-dBm:-117*/, \
    0x6 /* est_deci-dBm:-100 act_deci-dBm:-102*/, \
    0x6 /* est_deci-dBm:-90 act_deci-dBm:-102*/,  \
    0x7 /* est_deci-dBm:-80 act_deci-dBm:-89*/,   \
    0x8 /* est_deci-dBm:-70 act_deci-dBm:-78*/,   \
    0xa /* est_deci-dBm:-60 act_deci-dBm:-60*/,   \
    0xb /* est_deci-dBm:-50 act_deci-dBm:-53*/,   \
    0xd /* est_deci-dBm:-40 act_deci-dBm:-40*/,   \
    0xe /* est_deci-dBm:-30 act_deci-dBm:-34*/,   \
    0x10 /* est_deci-dBm:-20 act_deci-dBm:-23*/,  \
    0x12 /* est_deci-dBm:-10 act_deci-dBm:-14*/,  \
    0x15 /* est_deci-dBm:0 act_deci-dBm:-2*/,     \
    0x19 /* est_deci-dBm:10 act_deci-dBm:10*/,    \
    0x1d /* est_deci-dBm:20 act_deci-dBm:20*/,    \
    0x23 /* est_deci-dBm:30 act_deci-dBm:30*/,    \
    0x2a /* est_deci-dBm:40 act_deci-dBm:40*/,    \
    0x33 /* est_deci-dBm:50 act_deci-dBm:50*/,    \
    0x41 /* est_deci-dBm:60 act_deci-dBm:60*/,    \
    0x58 /* est_deci-dBm:70 act_deci-dBm:70*/,    \
    0x7d /* est_deci-dBm:80 act_deci-dBm:79*/,    \
  }

#ifdef __cplusplus
}
#endif

#endif
