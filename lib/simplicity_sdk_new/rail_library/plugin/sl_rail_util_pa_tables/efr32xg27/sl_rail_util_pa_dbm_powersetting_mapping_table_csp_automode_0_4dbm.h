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

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0x0 /* est_deci-dBm:-300 act_deci-dBm:-302*/,    \
    0x0 /* est_deci-dBm:-290 act_deci-dBm:-302*/,    \
    0x0 /* est_deci-dBm:-280 act_deci-dBm:-302*/,    \
    0x8000 /* est_deci-dBm:-270 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-260 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-250 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-240 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-230 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-220 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-210 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-200 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-190 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-180 act_deci-dBm:-277*/, \
    0x8000 /* est_deci-dBm:-170 act_deci-dBm:-277*/, \
    0x8001 /* est_deci-dBm:-160 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-150 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-140 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-130 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-120 act_deci-dBm:-161*/, \
    0x8001 /* est_deci-dBm:-110 act_deci-dBm:-161*/, \
    0x8002 /* est_deci-dBm:-100 act_deci-dBm:-108*/, \
    0x8002 /* est_deci-dBm:-90 act_deci-dBm:-108*/,  \
    0x8002 /* est_deci-dBm:-80 act_deci-dBm:-108*/,  \
    0x8003 /* est_deci-dBm:-70 act_deci-dBm:-78*/,   \
    0x8003 /* est_deci-dBm:-60 act_deci-dBm:-78*/,   \
    0x8004 /* est_deci-dBm:-50 act_deci-dBm:-58*/,   \
    0x8005 /* est_deci-dBm:-40 act_deci-dBm:-43*/,   \
    0x8006 /* est_deci-dBm:-30 act_deci-dBm:-33*/,   \
    0x8007 /* est_deci-dBm:-20 act_deci-dBm:-24*/,   \
    0x8009 /* est_deci-dBm:-10 act_deci-dBm:-12*/,   \
    0x800c /* est_deci-dBm:0 act_deci-dBm:-1*/,      \
    0x15 /* est_deci-dBm:10 act_deci-dBm:8*/,        \
    0x19 /* est_deci-dBm:20 act_deci-dBm:19*/,       \
    0x1e /* est_deci-dBm:30 act_deci-dBm:30*/,       \
    0x25 /* est_deci-dBm:40 act_deci-dBm:39*/,       \
    0x2e /* est_deci-dBm:50 act_deci-dBm:50*/,       \
    0x3b /* est_deci-dBm:60 act_deci-dBm:60*/,       \
    0x53 /* est_deci-dBm:70 act_deci-dBm:70*/,       \
    0x7f /* est_deci-dBm:80 act_deci-dBm:79*/,       \
  }

#ifdef __cplusplus
}
#endif

#endif
