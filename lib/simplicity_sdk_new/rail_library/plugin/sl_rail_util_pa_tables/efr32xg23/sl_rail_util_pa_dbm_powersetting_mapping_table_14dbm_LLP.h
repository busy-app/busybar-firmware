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

#ifndef __PA_DBM_POWERSETTING_MAPPING_EFR32XG23_H_
#define __PA_DBM_POWERSETTING_MAPPING_EFR32XG23_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SL_RAIL_PA_TABLE_0_NUM_VALUES  (34U)
#define SL_RAIL_PA_TABLE_0_STEP_DDBM   (10U)
#define SL_RAIL_PA_TABLE_0_MAX_POWER_DDBM     (-50)
#define SL_RAIL_PA_TABLE_0_MIN_POWER_DDBM     (-380)

#define SL_RAIL_PA_TABLE_0                           \
  {                                                  \
    0xc000 /* est_deci-dBm:-380 act_deci-dBm:-376*/, \
    0xc004 /* est_deci-dBm:-370 act_deci-dBm:-374*/, \
    0xc005 /* est_deci-dBm:-360 act_deci-dBm:-363*/, \
    0xc007 /* est_deci-dBm:-350 act_deci-dBm:-350*/, \
    0xc008 /* est_deci-dBm:-340 act_deci-dBm:-344*/, \
    0xc009 /* est_deci-dBm:-330 act_deci-dBm:-334*/, \
    0xc00b /* est_deci-dBm:-320 act_deci-dBm:-322*/, \
    0xc00c /* est_deci-dBm:-310 act_deci-dBm:-317*/, \
    0xc00e /* est_deci-dBm:-300 act_deci-dBm:-302*/, \
    0xc010 /* est_deci-dBm:-290 act_deci-dBm:-292*/, \
    0xc011 /* est_deci-dBm:-280 act_deci-dBm:-281*/, \
    0xc014 /* est_deci-dBm:-270 act_deci-dBm:-270*/, \
    0xc016 /* est_deci-dBm:-260 act_deci-dBm:-261*/, \
    0xc018 /* est_deci-dBm:-250 act_deci-dBm:-254*/, \
    0xc01c /* est_deci-dBm:-240 act_deci-dBm:-241*/, \
    0xc01f /* est_deci-dBm:-230 act_deci-dBm:-230*/, \
    0xc023 /* est_deci-dBm:-220 act_deci-dBm:-221*/, \
    0xc028 /* est_deci-dBm:-210 act_deci-dBm:-211*/, \
    0xc02c /* est_deci-dBm:-200 act_deci-dBm:-202*/, \
    0xc031 /* est_deci-dBm:-190 act_deci-dBm:-191*/, \
    0xc038 /* est_deci-dBm:-180 act_deci-dBm:-180*/, \
    0xc03e /* est_deci-dBm:-170 act_deci-dBm:-170*/, \
    0xc046 /* est_deci-dBm:-160 act_deci-dBm:-160*/, \
    0xc04f /* est_deci-dBm:-150 act_deci-dBm:-150*/, \
    0xc057 /* est_deci-dBm:-140 act_deci-dBm:-141*/, \
    0xc062 /* est_deci-dBm:-130 act_deci-dBm:-130*/, \
    0xc06e /* est_deci-dBm:-120 act_deci-dBm:-121*/, \
    0xc07c /* est_deci-dBm:-110 act_deci-dBm:-110*/, \
    0xc08c /* est_deci-dBm:-100 act_deci-dBm:-100*/, \
    0xc09d /* est_deci-dBm:-90 act_deci-dBm:-90*/,   \
    0xc0b0 /* est_deci-dBm:-80 act_deci-dBm:-80*/,   \
    0xc0c5 /* est_deci-dBm:-70 act_deci-dBm:-70*/,   \
    0xc0dd /* est_deci-dBm:-60 act_deci-dBm:-60*/,   \
    0xc0ed /* est_deci-dBm:-50 act_deci-dBm:-50*/,   \
  }

#ifdef __cplusplus
}
#endif

#endif
