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

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (50U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (5U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (0U)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-245)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x8000 /* est_deci-dBm:-245 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-240 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-235 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-230 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-225 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-220 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-215 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-210 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-205 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-200 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-195 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-190 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-185 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-180 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-175 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-170 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-165 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-160 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-155 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-150 act_deci-dBm:-245*/, \
    0x8000 /* est_deci-dBm:-145 act_deci-dBm:-245*/, \
    0x8001 /* est_deci-dBm:-140 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-135 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-130 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-125 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-120 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-115 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-110 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-105 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-100 act_deci-dBm:-144*/, \
    0x8001 /* est_deci-dBm:-95 act_deci-dBm:-144*/,  \
    0x8002 /* est_deci-dBm:-90 act_deci-dBm:-94*/,   \
    0x8002 /* est_deci-dBm:-85 act_deci-dBm:-94*/,   \
    0x8002 /* est_deci-dBm:-80 act_deci-dBm:-94*/,   \
    0x8002 /* est_deci-dBm:-75 act_deci-dBm:-94*/,   \
    0x8002 /* est_deci-dBm:-70 act_deci-dBm:-94*/,   \
    0x8003 /* est_deci-dBm:-65 act_deci-dBm:-69*/,   \
    0x8003 /* est_deci-dBm:-60 act_deci-dBm:-69*/,   \
    0x8003 /* est_deci-dBm:-55 act_deci-dBm:-69*/,   \
    0x8004 /* est_deci-dBm:-50 act_deci-dBm:-54*/,   \
    0x8004 /* est_deci-dBm:-45 act_deci-dBm:-54*/,   \
    0x8005 /* est_deci-dBm:-40 act_deci-dBm:-44*/,   \
    0x8006 /* est_deci-dBm:-35 act_deci-dBm:-37*/,   \
    0x8007 /* est_deci-dBm:-30 act_deci-dBm:-32*/,   \
    0x8009 /* est_deci-dBm:-25 act_deci-dBm:-26*/,   \
    0x800c /* est_deci-dBm:-20 act_deci-dBm:-20*/,   \
    0x800f /* est_deci-dBm:-15 act_deci-dBm:-17*/,   \
    0x800f /* est_deci-dBm:-10 act_deci-dBm:-17*/,   \
    0x800f /* est_deci-dBm:-5 act_deci-dBm:-17*/,    \
    0x800f /* est_deci-dBm:0 act_deci-dBm:-17*/,     \
  }

#ifdef __cplusplus
}
#endif

#endif
